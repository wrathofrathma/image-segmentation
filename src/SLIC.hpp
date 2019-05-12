#ifndef SLIC_HPP
#define SLIC_HPP
#include <SFML/Graphics.hpp> //Contains sf::Image, which is our image loader/writer.


#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include <cstdlib>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath> //std::abs std::sqrt std::pow pi

static const double deg_to_rad = M_PI / 180.0;

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
struct vertex_data {
	RGBA rgba;
	HSV hsv;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, vertex_data, boost::no_property> PIXEL_GRAPH;

class Slic {
  private:
    //Image data
    sf::Image image; ///< Proper image.
    std::string if_path; ///< Input file path.
    unsigned int num_cuts; ///< Number of graph cuts.
    unsigned int total_pixels; ///< Total pixels in the original image.
    unsigned int height; ///< Image height
    unsigned int width;  ///< Image width
    bool greyscale; ///< Did we convert to greyscale?

    //Graph data
    PIXEL_GRAPH graph; ///< Our active working graph.
    unsigned int vertex_count; ///< Our graph's vertex count.

    bool ready; ///< Did our file load correctly and are we ready to start working on it?
		bool graph_loaded; ///< Did we load our pixels to the graph?
    //Private utility functions.
    HSV rgb2hsv(RGBA rgb); ///< Converts a pixel's color data from RGBA format to HSV. This algorithm is an implementation of the one documented on wikipedia.
    double getColorDistance(HSV c1, HSV c2); ///< Returns the distance between two pixels in HSV colorspace.
		HSV getClusterAverage(std::vector<HSV> colors); ///< Returns the average color value of everything in the cluster.
  public:
    Slic(std::string filename); ///< Takes a file path input and loads the image.
    ~Slic();
    void writeToDisk(std::string out_dir); ///< Writes our graph to file.
    void convertToGreyscale(); ///< Converts our image to greyscale
    void loadToGraph(); ///< Loads the current data in pixel_data data pointer to our graph.
		void generateSuperpixels(unsigned int step, int weight);
};

#endif
