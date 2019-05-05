#ifndef GRAPH_HPP
#define GRAPH_HPP
#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <stack>
#include <ctime>
#include <cstdlib>
using namespace std;

enum color_t {
  white, //Before we start
  grey, //After we visit vertex but not before we've visited all adjancent vertices
  black //After we've visited vertex and all adjancent
};
//Comparison functions.
bool str_comp(std::string v, std::string w){
  if(v.compare(w)==0)
    return true;
  return false;
}

bool int_comp(int v, int w){
  if(v==w)
    return true;
  return false;
}


template <typename T>
class Graph {
  private:
    typedef bool (*comp_ptr)(T, T); //Pointer definition we'll use to hold our comparison function later.

    map<unsigned int, vector<unsigned int>> adj_nodes; //adjacency list
    vector<T> nodes; //Vertices
    unsigned int vertex_count;
    unsigned int edge_count;
    vector<pair<unsigned int, unsigned int>> edges; //Our edge list, for doing vertex cover calcs later.
    comp_ptr comp; //Our comparison algorithm
    bool digraph; //Boolean value for whether this is a directed graph or not.
    //Utility functions for DFS exploration and cycle determination.
    void DFSRec(unsigned int v_key, map<unsigned int, color_t> &colors);
    void DFSRec(unsigned int v_key, map<unsigned int, color_t> &colors, vector<unsigned int> &depart, int &time);
    bool cycleRec(unsigned int v, map<unsigned int, color_t> &colors, vector<bool> &recStack);
    bool isCyclic(); // Tests for cycles in the graph.
    void rDFS(int s, map<unsigned int, color_t> &colors, vector<unsigned int> &depart, int &time); //Print vertices discovered by an recursive DFS starting at node passed

  public:

    Graph(bool directed, bool (*comparison)(T v, T w));
    Graph(const Graph<T> &g1);
    ~Graph();
    void addVertex(T); //Add a vertex to the graph
    void addEdge(T, T); //Add an undirected edge to the graph
    void print(); //Print adjacency list of each vertex
    void printBFS(T, bool pdist=false); //Print vertices discovered by a BFS starting at node passed
    void DFS(T); //Print vertices discovered by an interative DFS starting at node passed
    void rDFS(T); //Print vertices discovered by an recursive DFS starting at node passed
    unsigned int getEdgeCount() const;
    unsigned int getVertexCount() const;
    void topSort(); //Iterative topological sort
    vector<unsigned int> topSort2(bool p, bool dag=true); //Recursive topological sort. Input whether to print, and whether we need it to be a DAG. Which is so I can reuse it later for another calc.
    bool isDAG();
    void transpose(); //Transposes the graph in place.
    void printFullyConnectedComponents(); //Prints each fully connected component list on new lines.
    void printUVertexColor(); // Uncovered vertex cover algorithm
    void printRVertexColor(); // Random vertex cover algorithm
    //Utility functions for getting specific values. They're all set to const so I can use it in my copy constructor.(Also because they should be.)
    bool isDigraph() const;
    void* getComp() const;
    vector<T> getNodes() const;
    map<unsigned int, vector<unsigned int>> getAdjNodeMap() const;
    vector<pair<unsigned int, unsigned int>> getEdges() const;
};

#include "Graph.cpp"
#endif
