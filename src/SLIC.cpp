#include "SLIC.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <chrono>
#include <cmath>
#include <boost/filesystem/convenience.hpp>

Slic::Slic(std::string filename){
  if_path = filename;
  std::cout << "Loading " << filename << " pixel data..." << std::endl;
  if(!image.loadFromFile(filename)){
    std::cout << "Problem loading filename: " << filename << std::endl;
    ready = false;
  }

  greyscale = false;
  ready = true;
  height = image.getSize().y;
  width = image.getSize().x;
  total_pixels = height * width;
  num_cuts = 0;
  std::cout << "Loaded " << width << "x" << height << " file " << filename << std::endl
    << "Total pixels: " << total_pixels << std::endl;
}

Slic::~Slic(){

}

//O(1) constant
double Slic::getColorDistance(HSV c1, HSV c2){
  //Remember, these are coming in as
  // Hue 0-360
  // Saturation 0-100
  // Value 0 - 100
  double h1 = c1.hue * deg_to_rad;
  double h2 = c2.hue * deg_to_rad;
  double s1 = c1.saturation/100.0;
  double s2 = c2.saturation/100.0;
  double v1 = c1.value / 100.0;
  double v2 = c2.value / 100.0;

  //We're going to project the color value into the HSV color space and then measure the distance.
  double p1 = std::pow(std::sin(h1)*s1*v1 - std::sin(h2)*s2*v2,2);
  double p2 = std::pow(std::cos(h1)*s1*v1 - std::cos(h2)*s2*v2,2);
  double p3 = std::pow(v1 - v2,2);
  double dist2 = p1 + p2 + p3; //Our distance squared.
  return sqrt(dist2);
}

//O(1) constant
HSV Slic::rgb2hsv(RGBA rgb){
	float MAX = std::max(std::max(rgb.r,rgb.g), rgb.b);
	float MIN = std::min(std::min(rgb.r,rgb.g), rgb.b);
	int hue=0;
	if(MAX==MIN)
		hue = 0;
	else if(MAX==rgb.r)
		hue = 60*((rgb.g - rgb.b)/(MAX-MIN));
	else if(MAX==rgb.g)
		hue = 60*(2 + (rgb.b - rgb.r)/(MAX-MIN));
	else if(MAX==rgb.b)
		hue = 60*(4 + (rgb.r - rgb.g)/(MAX-MIN));
	if(hue < 0)
		hue+=360;
	int saturation = 0;
	if(MAX==0)
		saturation = 0;
	else
		saturation = (MAX-MIN)/MAX * 100;
	int value = (MAX + MIN)/2.0 * 100;
	return HSV{hue,saturation,value};
}

// O(n) operation. Passes each pixel once.
void Slic::convertToGreyscale(){
  if(!ready)
    return;
  for(unsigned int i=0; i<height; i++){
    for(unsigned int j=0; j<width; j++){
      sf::Color pixel_color = image.getPixel(j,i);
      float gs = (pixel_color.r + pixel_color.g + pixel_color.b)/3; // Averages the pixel color in rgb. Which converts it to grey.
      image.setPixel(j,i, sf::Color(gs,gs,gs,255));
    }
  }
  greyscale = true;
}

void Slic::loadToGraph(){
  //Pixels are stored in a c style array in RGBA pixel format made of 8 bit integers.
  const sf::Uint8 *pixel_data = image.getPixelsPtr();
  for(unsigned int i=0; i<height; i++){
    for(unsigned int j=0; j<width; j++){
      //We're i units down height wise, and for each we have passed the width once. Then we're j units over on the current line.
      // 0 0 0 1
      // 1 0 2 0
      // 2 0 X 0
      // X is at spot (2,2), with a width of 4, and j of 2, we would have index = (2*4)+2 = 10. Which is correct.
      unsigned int index = (i*width) + j;
      //Extract our colors from the index
      sf::Uint8 r = pixel_data[4*index];
      sf::Uint8 g = pixel_data[4*index+1];
      sf::Uint8 b = pixel_data[4*index+2];
      sf::Uint8 a = pixel_data[4*index+3];
      //Convert to HSV colorspace.
      RGBA rgba = { int(r)/255.0, int(g)/255.0, int(b)/255.0, int(a)/255.0};
      HSV hsv = rgb2hsv(rgba);
      boost::add_vertex(vertex_color_t{rgba, hsv}, graph);
    }
  }
  graph_loaded = true;
}


void Slic::writeToDisk(std::string out_dir){
  if(!ready || graph_loaded){
    return;
  }
  //Create a new pixel array.
	sf::Uint8 out_pixels[total_pixels*4];
  const sf::Uint8 *pixel_data = image.getPixelsPtr();
	//Filling our pixel array with new colors based on our connected components vector.
	auto vp = boost::vertices(graph);
	for(auto iter = vp.first; iter!=vp.second; iter++){
		unsigned int index = *iter;

		out_pixels[4*index] = graph[index].rgba.r;
		out_pixels[4*index+1] = graph[index].rgba.g;
		out_pixels[4*index+2] = graph[index].rgba.b;
		out_pixels[4*index+3] = graph[index].rgba.a;
	}


  //Strip path from input
  std::filesystem::path input_path(if_path);
  std::string no_path = input_path.filename();
  //Strip extension from pathless input
  std::filesystem::path pathless_path(no_path);
  std::string base_path = pathless_path.stem();
  std::string extension = "";
  if(pathless_path.has_extension())
    extension += pathless_path.extension().string();

  //Create our output filename
  base_path += "_output";
  if(greyscale)
    base_path += "_greyscale";
  if(extension.size()>0)
    base_path += extension;
  std::string out_path = out_dir + base_path;

  //Create our image to write and write it.
  sf::Image out_image;
  out_image.create(width, height, out_pixels);
  out_image.saveToFile(out_path);
}
