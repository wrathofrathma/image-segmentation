
#include <SFML/Graphics.hpp> //Contains sf::Image, which is our image loader/writer.
#include <iostream>
#include <string>
#include <chrono> //Timing shit
#include <utility> //std::pair
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/filesystem/convenience.hpp>
#include <filesystem>
#include <cstdlib>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath> //std::abs std::sqrt std::pow pi

using namespace std;

struct HSV {
	int hue;
	int saturation;
	int value;
};

struct RGBA{
	double r;
	double g;
	double b;
	double a;
};

struct vertex_color_t {
	RGBA rgba;
	HSV hsv;
	bool bg;
};

double degToRad(int d){
	return d*(M_PI/180);
}

//Implementation of algorithm found and documented on wikipedia
HSV rgb2hsv(RGBA rgb){
	float MAX = max(max(rgb.r,rgb.g), rgb.b);
	float MIN = min(min(rgb.r,rgb.g), rgb.b);
	unsigned int hue=0;
	if(MAX==MIN){
		hue = 0;
	}
	else if(MAX==rgb.r){
		hue = 60*((rgb.g - rgb.b)/(MAX-MIN));
	}
	else if(MAX==rgb.g){
		hue = 60*(2 + (rgb.b - rgb.r)/(MAX-MIN));
	}
	else if(MAX==rgb.b){
		hue = 60*(4 + (rgb.r - rgb.g)/(MAX-MIN));
	}
	if(hue < 0)
		hue+=360;

	int saturation = 0;
	if(MAX==0)
		saturation = 0;
	else{
		saturation = (MAX-MIN)/MAX * 100;
	}

	int value = (MAX + MIN)/2.0 * 100;

	return HSV{hue,saturation,value};
}

sf::Color hsv2rgb(HSV){
	return sf::Color::Black;
}

//Compares two colors and returns whether they're close in color.
bool color_comp(RGBA c1, RGBA c2, float K){

	//This comparison algorithm is an implementation of the low-cost approximation described here: https://en.wikipedia.org/wiki/Color_difference
	//Color deltas.
	int rd = (c1.r*255 - c2.r*255);
	int gd = (c1.g*255 - c2.g*255);
	int bd = (c1.b*255 - c2.b*255);

	float rhat = (c1.r + c2.r)/256;

	//Components of the sqrt algorithm
	float rcomp = (2+rhat) * pow(rd,2);
	float gcomp = 4*pow(gd,2);
	float bcomp = (2+(255-rhat)/256) * pow(bd, 2);
	float color_delta = sqrt(rcomp + gcomp + bcomp);

	if(color_delta < K)
		return true;
	return false;
}

bool hcolor_comp(HSV c1, HSV c2, double delta){
	//Remember, these are coming in as
	// Hue 0-360
	// Saturation 0-100
	// Value 0 - 100
	double h1 = degToRad(c1.hue);
	double h2 = degToRad(c2.hue);
	double s1 = c1.saturation/100.0;
	double s2 = c2.saturation/100.0;
	double v1 = c1.value / 100.0;
	double v2 = c2.value / 100.0;

	//We're going to project the color value into the HSV color space and then measure the distance.
	double p1 = pow(sin(h1)*s1*v1 - sin(h2)*s2*v2,2);
	double p2 = pow(cos(h1)*s1*v1 - cos(h2)*s2*v2,2);
	double p3 = pow(v1 - v2,2);
	double dist2 = p1 + p2 + p3; //Our distance squared.
	double dist = sqrt(dist2);

	if(dist < delta)
		return true;
	return false;
}

// bool hcolor_comp(HSV c1, HSV c2, double delta){
// 	//Remember, these are coming in as
// 	// Hue 0-360
// 	// Saturation 0-100
// 	// Value 0 - 100
//
// 	double v1 = c1.value / 100.0;
// 	double v2 = c2.value / 100.0;
// 	double dist = abs(v1-v2);
// 	if(dist < delta)
// 		return true;
// 	return false;
// }
//This function generates a component color based on the total components & component id.
//For now we'll assume there are less than 255 components and scale up later if it becomes a problem.
sf::Color getComponentColor(int component_id){
	int some_prime = 31907;
	srand(component_id * some_prime);
	int r = rand() % 256;
	int g = rand() % 256;
	int b = rand() % 256;
	return sf::Color(r,g,b,255);
}
int main(int argc, char **argv){

	if(argc!=3){
		std::cout << "Please use syntax ./prog image_name delta" << std::endl;
		return -1;
	}
	std::string input_file = argv[1];
	//Load our image.
	sf::Image img;
	if(!img.loadFromFile(input_file)){
		return -1;
	}
	double delta = stod(string(argv[2]));
	//Get image dimensions.
	unsigned int height = img.getSize().y;
	unsigned int width = img.getSize().x;
	size_t total_pixels = height * width;

	cout << "Loaded " << width << "x" << height << " file " << argv[1] << endl;
	cout << "Total pixels: " << total_pixels << endl;

	bool bgcalc = false;
	float K = 50;
	float BK = K;
	float ar=0;
	float ag=0;
	float ab=0;
	for(unsigned int i=0; i<height; i++){
		for(unsigned int j=0; j<width; j++){
			//unsigned int index = (i*width) + j;
			sf::Color pixColor = img.getPixel(j,i);
			float g = (pixColor.r + pixColor.g + pixColor.b)/3;
			 ar+=pixColor.r/255.0;
			 ag+=pixColor.g/255.0;
			 ab+=pixColor.b/255.0;
			// ar+=g/255.0;
			// ag+=g/255.0;
			// ab+=g/255.0;
			// img.setPixel(j,i, sf::Color(g,g,g,255));
		}
	}
	cout << "Converted to greyscale" << endl;
	ar/=total_pixels;
	ag/=total_pixels;
	ab/=total_pixels;
	RGBA avg_color = { ar,ag,ab,1.0};
	cout << "Average pixel color(prgb): " << ar << " " << ag << " " << ab << endl;
	HSV avg_hsv = rgb2hsv(avg_color);
	cout << "Average pixel color(hsv): " << avg_hsv.hue << " " << avg_hsv.saturation << " " << avg_hsv.value << endl;

	//Get a pointer to the c style string of data.
	const sf::Uint8* pixels = img.getPixelsPtr();


	//Generate our graph object type and link our color property to vertex properties.
	typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, vertex_color_t, boost::no_property> Graph;

	//Our actual graph object is going to have a number of vertices = total_pixels.
	//Graph graph(total_pixels);
	Graph graph;

	//Pixels are stored in a c style array in RGBA pixel format made of 8 bit integers.
	//Let's load them into our graph.
	auto start = chrono::high_resolution_clock::now();
	for(unsigned int i=0; i<height; i++){
		for(unsigned int j=0; j<width; j++){
			//We're i units down height wise, and for each we have passed the width once. Then we're j units over on the current line.
			// 0 0 0 1
			// 1 0 2 0
			// 2 0 X 0
			// X is at spot (2,2), with a width of 4, and j of 2, we would have index = (2*4)+2 = 10. Which is correct.
			unsigned int index = (i*width) + j;
			//Extract our colors from the index
			sf::Uint8 r = pixels[4*index];
			sf::Uint8 g = pixels[4*index+1];
			sf::Uint8 b = pixels[4*index+2];
			sf::Uint8 a = pixels[4*index+3];
			RGBA rgba = { int(r)/255.0, int(g)/255.0, int(b)/255.0, int(a)/255.0};
			HSV hsv = rgb2hsv(rgba);
			//Stuff the data into our graph.
			bool bbb = false;
			// if(color_comp(rgba, avg_color, BK))
			// 	bbb=true;
			if(hcolor_comp(hsv, avg_hsv, delta))
				bbb=true;
			boost::add_vertex(vertex_color_t{rgba, hsv, bbb}, graph);
		}
	}

	auto end = chrono::high_resolution_clock::now();
	chrono::duration<double,std::milli> elapsed = end-start;
	cout << "Took : " << elapsed.count() << "ms to insert vertex colors" << endl;

	//Now that we have our vertex colors filled in, we need to create edges based on whether they have a similar color.
	//We defined bool color_comp(sf::Color, sf::Color) up top with some parameters to vary.
	start = chrono::high_resolution_clock::now();
	// for(unsigned int i=0; i<height; i++){
	// 	for (unsigned int j=0; j<width; j++){
	// 		unsigned int index = (i*width) + j;
	// 		//We cast i & j to ints because our test will be incorrect if we go below 0 on unsigned since it wraps
	// 		if((int)(i)-1 > 0){
	// 			int up = ((i-1)*width + j);
	// 			// if(graph[index].bg==graph[up].bg)
	// 				if(color_comp(graph[index].rgba, graph[up].rgba,K))
	// 					boost::add_edge(index, up, graph);
	// 		}
	// 		if(i+1 < height){
	// 			int down = ((i+1)*width + j);
	// 			// if(graph[index].bg==graph[down].bg)
	// 				if(color_comp(graph[index].rgba, graph[down].rgba, K))
	// 					boost::add_edge(index, down, graph);
	// 		}
	// 		if((int)(j)-1 > 0){
	// 			int left = ((i)*width + j-1);
	// 			// if(graph[index].bg==graph[left].bg)
	// 				if(color_comp(graph[index].rgba, graph[left].rgba, K))
	// 					boost::add_edge(index, left, graph);
	// 		}
	// 		if(j+1 < width){
	// 			int right = ((i)*width + j+1);
	// 			// if(graph[index].bg==graph[right].bg)
	// 				if(color_comp(graph[index].rgba, graph[right].rgba, K))
	// 					boost::add_edge(index, right, graph);
	// 		}
	// 	}
	// }
	for(unsigned int i=0; i<height; i++){
		for (unsigned int j=0; j<width; j++){
			unsigned int index = (i*width) + j;
			//We cast i & j to ints because our test will be incorrect if we go below 0 on unsigned since it wraps
			if((int)(i)-1 > 0){
				int up = ((i-1)*width + j);
				if(bgcalc){
					if(graph[index].bg && graph[up].bg)
						boost::add_edge(index, up, graph);
				}
				if(hcolor_comp(graph[index].hsv, graph[up].hsv, delta))
					boost::add_edge(index, up, graph);
			}
			if(i+1 < height){
				int down = ((i+1)*width + j);
				if(bgcalc){
					if(graph[index].bg && graph[down].bg)
						boost::add_edge(index, down, graph);
				}
				if(hcolor_comp(graph[index].hsv, graph[down].hsv, delta))
					boost::add_edge(index, down, graph);
			}
			if((int)(j)-1 > 0){
				int left = ((i)*width + j-1);
				if(bgcalc){
					if(graph[index].bg && graph[left].bg)
						boost::add_edge(index, left, graph);
				}
				if(hcolor_comp(graph[index].hsv, graph[left].hsv, delta))
					boost::add_edge(index, left, graph);
			}
			if(j+1 < width){
				int right = ((i)*width + j+1);
				if(bgcalc){
					if(graph[index].bg && graph[right].bg)
						boost::add_edge(index, right, graph);
				}
				if(hcolor_comp(graph[index].hsv, graph[right].hsv, delta))
					boost::add_edge(index, right, graph);
			}
		}
	}
	end = chrono::high_resolution_clock::now();
	elapsed = end-start;
	cout << "Took " << elapsed.count() << "ms to add " << boost::num_edges(graph) << " edges" << endl;

	//Detect our components. Theoretically, the only things connected should be similar.
	//Our component vector should be filled with integer ids of which component an individual vertex belongs to now.
	std::vector<int> component(total_pixels);
	size_t num_components = boost::connected_components(graph, &component[0]);
	cout << "Number of components detected: " << num_components << endl;
	// for(unsigned int i = 0; i<num_components; i++){
	// 	int count = std::count(component.begin(), component.end(), i);
	// 	cout << "Number of IDs for " << i << " : " << count << endl;
	// }
	//Write our output to a file. Let's do this by just going through the graph and creating a new c_string.
	//Create and fill our pixel array.
	sf::Uint8 out_pixels[total_pixels*4];

	//Filling our pixel array with new colors based on our connected components vector.
	auto vp = boost::vertices(graph);
	for(auto iter = vp.first; iter!=vp.second; iter++){
		unsigned int index = *iter;
		//We use our getComponentColor(int,int) defined at the top to generate a new color for our component.
		//The function is a simple function of the component id & number of components. (Let's hope we don't have something over 255 types for simplicity sake)
		sf::Color index_color = getComponentColor(component[index]);
		if(graph[index].bg && bgcalc)
			index_color = sf::Color::Black;
		out_pixels[4*index] = index_color.r;
		out_pixels[4*index+1] = index_color.g;
		out_pixels[4*index+2] = index_color.b;
		out_pixels[4*index+3] = index_color.a;
	}

	//Let's use std::filesystem to extract our file extension.
	std::filesystem::path out_path(input_file);
	std::string out_filename = "output";
	if(out_path.has_extension()){
		out_filename += out_path.extension().string();
	}

	//Proper writing to disk.
  sf::Image out_image;
	out_image.create(width, height, out_pixels);
	out_image.saveToFile(out_filename);
	img.saveToFile("grey.jpg");
	return 0;
}
