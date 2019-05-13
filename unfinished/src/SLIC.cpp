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
	float hue=0;

  float eps = 0.00001;
	if(std::abs(MAX-MIN)<eps){
		hue = 0;
  }
	else if(std::abs(MAX-rgb.r)<eps){
		hue = 60*((rgb.g - rgb.b)/(MAX-MIN));
  }
	else if(std::abs(MAX-rgb.g)<eps){
		hue = 60*(2 + (rgb.b - rgb.r)/(MAX-MIN));
  }
	else if(std::abs(MAX-rgb.b)<eps){
		hue = 60*(4 + (rgb.r - rgb.g)/(MAX-MIN));
  }
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
      boost::add_vertex(vertex_data{rgba, hsv, std::vector<int>()}, graph);
    }
  }
  graph_loaded = true;
}
#include <cstdlib>
#include <ctime>
sf::Color getComponentColor(int component_id){
	int some_prime = 31907;
	srand(component_id * some_prime);
	int r = rand() % 256;
	int g = rand() % 256;
	int b = rand() % 256;
	return sf::Color(r,g,b,255);
}

struct rgb{
  double r;
  double g;
  double b;
};
//I legit yoinked this one from online.
RGBA hsv2rgb(HSV in)
{
    double      hh, p, q, t, ff;
    long        i;
    RGBA         out;
    in.saturation /= 100.0;
    in.value /= 100.0;
    if(in.saturation <= 0.0) {       // < is bogus, just shuts up warnings
        out.r = in.value;
        out.g = in.value;
        out.b = in.value;
        return out;
    }
    hh = in.hue;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.value * (1.0 - in.saturation);
    q = in.value * (1.0 - (in.saturation * ff));
    t = in.value * (1.0 - (in.saturation * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.value;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.value;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.value;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.value;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.value;
        break;
    case 5:
    default:
        out.r = in.value;
        out.g = p;
        out.b = q;
        break;
    }
    out.a = 255;
    return out;
}

void Slic::writeToDisk(std::string out_dir){
  if(!ready || !graph_loaded){
    std::cout << "Shit isn't ready:" << std::endl;
    return;
  }
  //Create a new pixel array.
	sf::Uint8 out_pixels[total_pixels*4];
  // const sf::Uint8 *pixel_data = image.getPixelsPtr();
	//Filling our pixel array with new colors based on our connected components vector.
	auto vp = boost::vertices(graph);
  std::cout << "Writing to disk" << std::endl;
	for(auto iter = vp.first; iter!=vp.second; iter++){
		unsigned int index = *iter;
    // std::cout << graph[index].cluster_ids[0] << std::endl;
    sf::Color col = getComponentColor(graph[index].cluster_ids[0]);
    // std::cout << graph[index].cluster_ids[0] << col.r << " " << col.g << std::endl;

    out_pixels[4*index] = col.r;
		out_pixels[4*index+1] = col.g;
		out_pixels[4*index+2] = col.b;
		out_pixels[4*index+3] = 255;
		// out_pixels[4*index] = graph[index].rgba.r;
		// out_pixels[4*index+1] = graph[index].rgba.g;
		// out_pixels[4*index+2] = graph[index].rgba.b;
		// out_pixels[4*index+3] = graph[index].rgba.a;
    // HSV hs = clusters[graph[index].cluster_ids[0]].color;
    // RGBA col = hsv2rgb(hs);
    // out_pixels[4*index] = col.r*255;
    // out_pixels[4*index+1] = col.g*255;
    // out_pixels[4*index+2] = col.b*255;
    // out_pixels[4*index+3] = 255;
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

void Slic::cleanEdges(){
  auto vp = boost::vertices(graph);
  //For every vertex/pixel.
  for(auto iter = vp.first; iter!=vp.second; iter++){
    //Check it against all associated cluster ids.
    edge_data best_fit = { -1 , -1 };
    if(graph[*iter].cluster_ids.size()==0)
      continue;
    for(int id : graph[*iter].cluster_ids){
      //It might be less costly to just remove each edge as we check it then add the last one. The other option is to loop twice.
      //Convert cluster coordinates to an indice
      int ci = clusters[id].cpos.y * width + clusters[id].cpos.x;

      auto e = boost::edge(*iter, ci, graph).first;
      //Find the best fitting edge and remove all of the rest.
      if(best_fit.distance == -1){
        best_fit = graph[e];
        boost::remove_edge(e,graph);
      }
      else if(best_fit.distance > graph[e].distance){
        best_fit = graph[e];
        boost::remove_edge(e,graph);
      }
    }
    graph[*iter].cluster_ids.clear();
    //Reinsert the best edge. wasteful but fuck it.
    int ci = clusters[best_fit.cluster_id].cpos.y * width + clusters[best_fit.cluster_id].cpos.x;
    boost::add_edge(*iter, ci, best_fit, graph);
    graph[*iter].cluster_ids.push_back(best_fit.cluster_id);
  }
}

//O(n) - We hit every pixel or vertex in this algorithm.
void Slic::updateClusterAverages(){
  // 1. Generate connected component vector to find which
  // 2.
  if(!graph_loaded)
    return;
  auto vp = boost::vertices(graph);
  //Containers to store shit in
  std::vector<HSV> colors(clusters.size(),{0,0,0});
  std::vector<int> color_count(clusters.size(),0);
  //Loop through and add up all of the color values for each cluster.
  for(auto iter = vp.first; iter!=vp.second; iter++){
    auto v = graph[*iter];
    if(v.cluster_ids.size()==0){
      return;
    }
    colors[v.cluster_ids[0]].hue+= v.hsv.hue;
    colors[v.cluster_ids[0]].saturation += v.hsv.saturation;
    colors[v.cluster_ids[0]].value += v.hsv.value;
    color_count[v.cluster_ids[0]]++;
  }
  //since we used the cluster id as the index, we can loop through just once to average it all and set it.
  for(unsigned int i=0; i<clusters.size(); i++){
    if(color_count[i]!=0){
      clusters[i].color.hue = colors[i].hue / color_count[i];
      clusters[i].color.saturation = colors[i].saturation / color_count[i];
      clusters[i].color.value = colors[i].value / color_count[i];
    }
  }
}

bool Slic::inBounds(unsigned int x, unsigned int y){
  //Can't be less than 0 since unsigned.
  //If our width is 10, then index 9 is on the edge.
//  std::cout << "Checking if : " << x << " " << y << " - " << width << " " << height << std::endl;
  if(x >= width || y >= height)
    return false;
  return true;
}
bool Slic::isEdge(unsigned int x, unsigned int y){
  if(x == width-1 || y == height-1)
    return true;
  if(x == 0 || y == 0)
    return true;
  return false;
}
void Slic::generateSuperpixels(unsigned int K, unsigned int error){
  this->K = K;
  unsigned int S = std::sqrt(total_pixels / K);
  std::cout << "S Value: " << S << " " << width << " " << height << std::endl;
  //Generate cluster centers.
  for(unsigned int i=0; i*S < height; i++){
    for(unsigned int j=0; j*S < width; j++){
      int x = j*S; // We're just j*S units over.
      int y = i*S; //We're i*S units down
      int indice = y*width + x;
      if(inBounds(x,y)){
        //Get cluster center color.
        HSV hsv = graph[indice].hsv;
        clusters.push_back(Cluster{hsv,vec2<int>{x,y}, clusters.size()});
        std::cout << "Cluster center(" << clusters.size() << "): " << x << " " << y << " | Color: " << hsv.hue << " " << hsv.saturation << " " << hsv.value <<  std::endl;
      }
    }
  }
  //todo - make these slightly more accurate and move off of edges.

  //So every pass we're going to do the following
  //Check every pixel around the cluster centers in a radius of 2S.
  //Draw edges between detected pixels and the cluster centers.
  //Clean edges
  //Calculate cluster color

  while(clusterPass(S*2,error)){
    /*
    How's it going today?
    For me...I just wrote almost 10000 lines of code for a graphics project that probably won't get a B and now I'm cramming this on may 12th and it's 9pm.
    Also it's my birthday yesterday and I think I spent it on caffeine and programming. Honestly not too bad. I got to learn a lot about opengl.
    I just started getting into geometry shaders yesterday and water stuff too. My water is super ghetto, I have like 3 implementations lmao. the first one was a
    mesh that I pulled the vertices on a sinfunction using elapsed time to simulate waves. I don't have much faith in this new algorithm, but I did do some research.
    Hopefully it's sufficient. I kinda wish I found a way to do graph cuts using boost graph library. I think if I had like 2 more days I could do this projecvt so much better.
    I'd probably do my own graph library and probably auto remove edges lmao.
    */
  }

}

bool Slic::clusterPass(int radius, unsigned int error){
  //For each cluster
  for(Cluster cluster : clusters){
    //Check every pixel in a square radius around it.
    for(int y=-radius; y<=radius; y++){
      for(int x=-radius; x<=radius; x++){
        int xpos = cluster.cpos.x + x;
        int ypos = cluster.cpos.y + y;
        if(inBounds(xpos,ypos)){
          int indice1 = ypos*width + xpos;
          int indice2 = cluster.cpos.y * width + cluster.cpos.x;
          //Add edge
          boost::add_edge(indice1,indice2, { getColorDistance(graph[indice1].hsv, graph[indice2].hsv), cluster.id},graph);
          //Update our node cluster id list.
          graph[indice1].cluster_ids.push_back(cluster.id);
        }
      }
    }
  }
  cleanEdges();
  updateClusterAverages();
  return updateClusterPositions(error);
}

//Updates cluster positions and calculates the error threshhold for stopping our loop.
bool Slic::updateClusterPositions(unsigned int error){
  if(!graph_loaded)
    return false;
  auto vp = boost::vertices(graph);
  //Containers to store shit in
  std::vector<vec2<int>> pos(clusters.size(),{0,0});
  std::vector<int> count(clusters.size(),0);
  //Loop through all of the vertices
  for(auto iter = vp.first; iter!=vp.second; iter++){
    auto v = graph[*iter];
    pos[v.cluster_ids[0]].x += clusters[v.cluster_ids[0]].cpos.x;
    pos[v.cluster_ids[0]].y += clusters[v.cluster_ids[0]].cpos.y;
    count[v.cluster_ids[0]]++;
  }
  for(unsigned int i = 0; i<count.size(); i++){
    if(count[i]!=0){
      //Compute error vector.
      vec2<int> npos;
      npos.x = pos[i].x / count[i];
      npos.y = pos[i].y / count[i];
      double l1 = std::sqrt(std::pow((npos.x),2) + std::pow(npos.y,2) + pow(clusters[i].color.hue,2) + pow(clusters[i].color.saturation,2) + pow(clusters[i].color.value,2));
      double l2 = std::sqrt(std::pow((clusters[i].cpos.x),2) + std::pow(clusters[i].cpos.y,2)+ pow(clusters[i].color.hue,2) + pow(clusters[i].color.saturation,2) + pow(clusters[i].color.value,2));

       std::cout << "v1: " << npos.x << " " << npos.y << " v2 " << clusters[i].cpos.x << " " << clusters[i].cpos.y << std::endl;
       if((l1-l2) < error && (npos.x !=0 && npos.y!=0)){
         std::cout << "Error distance reached: " << l1-l2 << std::endl;
         return false;
       }
      clusters[i].cpos = npos;
    }
    else{
      std::cout << "Sir we've encountered a critical error. DIV/0. Somehow there's a pixel with no shit associated." << std::endl;
    }
  }
  return true;
}
