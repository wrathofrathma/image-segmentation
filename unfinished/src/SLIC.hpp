#ifndef SLIC_HPP
#define SLIC_HPP
#include <SFML/Graphics.hpp> //Contains sf::Image, which is our image loader/writer.
#include <list>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

#include <cstdlib>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <cmath> //std::abs std::sqrt std::pow pi

static const double deg_to_rad = M_PI / 180.0;

struct HSV {
	double hue;
	double saturation;
	double value;

};



struct RGBA{
	double r;
	double g;
	double b;
	double a;
};

//Vertex data is simple, we just need HSV values for comparison, and rgba values to write later if we want to restore the original image without conversion.
struct vertex_data {
	RGBA rgba;
	HSV hsv;
	std::vector<int> cluster_ids; ///< ID of the cluster in the vector we're associated with. -1 is unassociated.
};

struct edge_data {
	double distance;
	int cluster_id;
};

//I am so used to using glm::vec2 at this point that not having it bothers me.
template <typename T>
struct vec2 {
	T x;
	T y;
};

//For our clusters, we don't need anything too crazy. We just need to store the HSV color value, and the x/y coordinates(so we can )
struct Cluster {
	HSV color; ///Average color of the cluster.
	vec2<int> cpos; ///Center position of the cluster.
	int id;
};

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, vertex_data, edge_data> PIXEL_GRAPH;

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

		//Class flags.
    bool ready; ///< Did our file load correctly and are we ready to start working on it?
		bool graph_loaded; ///< Did we load our pixels to the graph?
    //Private utility functions.
    HSV rgb2hsv(RGBA rgb); ///< Converts a pixel's color data from RGBA format to HSV. This algorithm is an implementation of the one documented on wikipedia.
    double getColorDistance(HSV c1, HSV c2); ///< Returns the distance between two pixels in HSV colorspace.

		void updateClusterAverages(); ///< Updates the cluster average colors by using a connected components algorithm to determine which pixels are associated, then averaging.
		void cleanEdges(); ///< Removes extra edges from each vertex.
		std::vector<Cluster> clusters; ///< Vector of our clusters in the image. It's strictly used for the super pixel routine.
		bool isEdge(unsigned int x, unsigned int y);
		bool inBounds(unsigned int x, unsigned int y);

		bool clusterPass(int radius, unsigned int error);
		bool updateClusterPositions(unsigned int error);

		unsigned int K; ///< Number of initial clusters.
		unsigned int S; ///< Grid intervals
  public:
    Slic(std::string filename); ///< Takes a file path input and loads the image.
    ~Slic();
    void writeToDisk(std::string out_dir); ///< Writes our graph to file.
    void convertToGreyscale(); ///< Converts our image to greyscale
    void loadToGraph(); ///< Loads the current data in pixel_data data pointer to our graph.
		void generateSuperpixels(unsigned int K, unsigned int error);
};

#endif
