template<typename T>
Graph<T>::Graph(bool directed, bool (*comparison)(T v, T w)){
  vertex_count = 0;
  edge_count = 0;
  comp = comparison;
  digraph = directed;
}
//Copy constructor
template <typename T>
Graph<T>::Graph(const Graph<T> &g1){
  vertex_count = g1.getVertexCount();
  edge_count = g1.getEdgeCount();
  comp = (comp_ptr)g1.getComp(); //This one made me happy. It's been like 10 years since I had to use void pointers for something besides threads.
  digraph = g1.isDigraph();
  adj_nodes = g1.getAdjNodeMap();
  nodes = g1.getNodes();
  edges = g1.getEdges();
}

template <typename T>
vector<T> Graph<T>::getNodes() const{
  return nodes;
}

//Reverse all the edges by reversing the adjacency lists.
template <typename T>
void Graph<T>::transpose() {
  //Generate new adjacency map
  map<unsigned int, vector<unsigned int>> adj;
  for(unsigned int i=0; i<nodes.size(); i++){
    adj.insert(pair<unsigned int, vector<unsigned int>>(i, vector<unsigned int>()));
  }
  //For every node in our original map
  for(unsigned int v=0; v<nodes.size(); v++){
    //For every adjancent node in original map
    for(unsigned int u : adj_nodes[v]){
      //add to our new adjacency map as reverse
      adj[u].push_back(v);
    }
  }
  adj_nodes.clear();
  adj_nodes = adj;
}

template <typename T>
vector<pair<unsigned int, unsigned int>> Graph<T>::getEdges() const{
  return edges;
}
template <typename T>
bool Graph<T>::isDigraph() const{
  return digraph;
}

template <typename T>
void* Graph<T>::getComp() const{
  return (void *)comp;
}

template <typename T>
map<unsigned int, vector<unsigned int>> Graph<T>::getAdjNodeMap() const{
  return adj_nodes;
}

template <typename T>
Graph<T>::~Graph(){

}

template <typename T>
void Graph<T>::addVertex(T v){
  //Already exists if this is true
  for(T w : nodes){
    if(comp(v, w))
      return;
  }
  int i = nodes.size(); //The array position v will occupy
  nodes.push_back(v);
  adj_nodes.insert(pair<unsigned int, vector<unsigned int>>(i, vector<unsigned int>()));
  vertex_count++;
}

template <typename T>
void Graph<T>::addEdge(T v, T w){
  //Don't add if it's the same
  if(comp(v,w))
    return;
  //Finding keys for both vertices and seeing if they even exist.
  bool f1 = false;
  bool f2 = false;
  unsigned int vk;
  unsigned int wk;
  for(unsigned int i=0; i<nodes.size(); i++){
    if(comp(v,nodes[i])){
      f1 = true;
      vk = i;
      break;
    }
  }
  for(unsigned int i=0; i<nodes.size(); i++){
    if(comp(w,nodes[i])){
      f2 = true;
      wk = i;
      break;
    }
  }
  //If one of the vertices don't exist, then we can't add an edge.
  if(!f1 || !f2)
    return;

  //Check if they already have paths between them
  bool wv = false;
  bool vw = false;

  for(unsigned int i : adj_nodes[vk]){
    if(i==wk){
      vw = true;
      break;
    }
  }
  for(unsigned int i : adj_nodes[wk]){
    if(i==vk){
      wv = true;
      break;
    }
  }
  //Directed graph & vw exists.
  if(digraph && vw)
    return;
  //if both links exist, we don't do anything.
  else if(vw && wv)
    return;
  else if(!vw && !wv)
    edges.push_back(pair<unsigned int, unsigned int>(vk, wk)); // We only use edges in our calc later for undirected graph vertex covering. So we only need the undirected version.


  //Insert into one or both.
  if(!vw){
    adj_nodes[vk].push_back(wk);
  }
  if(!wv && !digraph){
    adj_nodes[wk].push_back(vk);
  }
  edge_count++;
}

template <typename T>
void Graph<T>::print(){
  cout << "HEAD     |     Adjacency list" << endl;

  for(unsigned int i = 0; i<nodes.size(); i++){
    cout << nodes[i] << "     |     ";
    for(int j : adj_nodes[i]){
      cout << nodes[j] << " ";
    }
    cout << endl;
  }
}

/*
BFS(s)
  color[s] = gray
  d[s] = 0
  ENQUEUE(Q, s)
  WHILE Q not empty DO
    DEQUEUE(Q, u)
    FOR (u, v) ∈ E DO
      IF color[v] = white THEN
        color[v] = gray
        d[v] = d[u] + 1
        parent[v] = u
        ENQUEUE(Q, v)
      END IF
      color[u] = black
    END FOR
  END WHILE

1. Instead of assigning a color to each node (possible but cumbersome, with modification to
the graph representation), use a std::map to store the colors, parents, and distances of each
vertex (as in the algorithm above). For example, the map for colors might be defined as:
std::map<int, color_t> where color_t is an enum to hold values for white, black and
gray.

2. Use your Heap Queue from an earlier lab or the std::queue<int> data structure to control
the exploration of the graph.
*/

template <typename T>
void Graph<T>::printBFS(T s, bool pdist){
  queue<unsigned int> q;

  int sk;
  bool sk_found = false;
  //Generate color map && find node key
  map<unsigned int, color_t> colors;
  for(unsigned int i=0; i<nodes.size(); i++){
    colors.insert(pair<unsigned int, color_t>(i, white));
    if(comp(s, nodes[i])){
      sk_found = true;
      sk = i;
    }
  }

  //Node doesn't exist in our graph.
  if(!sk_found)
    return;

  //Setting up for the BFS search
  map<unsigned int, unsigned int> dist;
  map<unsigned int, unsigned int>::iterator dit;
  colors[sk] = grey;
  dist[sk] = 0;
  q.push(sk);

  cout << "-----------------------------------------" << endl;
  cout << "BFS Search " << endl;
  while(q.size()!=0){
    unsigned int v = q.front();
    cout << nodes[v] << " ";
    q.pop();
    //Visit adjacency list of v
    for(unsigned int u : adj_nodes[v]){
      if(colors[u] == white){
        colors[u] = grey;
        dist[u] = dist[v] + 1;

        q.push(u);
      }
    }

  }
  cout << endl;
  cout << "-----------------------------------------" << endl;
  if(pdist){
    cout << "Distance of each node from node " << s << endl;
    cout << "Node | Dist " << endl;
    for(dit = dist.begin(); dit!=dist.end(); dit++){
      cout << nodes[dit->first] << "   |   " << dit->second << endl;
    }
  }
}

//Recursive component of DFS. Input is teh vertex key, and color map.
template <typename T>
void Graph<T>::DFSRec(unsigned int v_key, map<unsigned int, color_t> &colors){
  //Mark it visited
  colors[v_key] = grey;
  cout << nodes[v_key] << " ";
  //Visit all neighbors
  for(unsigned int u : adj_nodes[v_key]){
    if(colors[u]==white){
      DFSRec(u, colors);
    }
  }
}
//Recurisve component of DFS. Takes in vertex key, color map, a vector to hold the timing of when we visit the node, and the actual time integer.
template <typename T>
void Graph<T>::DFSRec(unsigned int v, map<unsigned int, color_t> &colors, vector<unsigned int> &depart, int &time){
  //Mark it visited then visit all neighbors recursively if it hasn't been visited yet
  colors[v] = grey;
  for(unsigned int u : adj_nodes[v]){
    if(colors[u]==white){
      DFSRec(u, colors, depart, time);
    }
  }
  //Mark when we leave.
  depart[time] = v;
  time++;
}
//DFS
template <typename T>
void Graph<T>::rDFS(T s){
  int sk;
  bool sk_found = false;
  //Generate color map && find node key
  map<unsigned int, color_t> colors;
  for(unsigned int i=0; i<nodes.size(); i++){
    colors.insert(pair<unsigned int, color_t>(i, white));
    if(comp(s, nodes[i])){
      sk_found = true;
      sk = i;
    }
  }
  //Node doesn't exist in our graph.
  if(!sk_found)
    return;

  DFSRec(sk, colors);
  cout << endl;
}
template <typename T>
void Graph<T>::rDFS(int s, map<unsigned int, color_t> &colors, vector<unsigned int> &depart, int &time){
  //No setup really needed.
  DFSRec(s, colors, depart, time);
}

template<typename T>
bool Graph<T>::cycleRec(unsigned int v, map<unsigned int, color_t> &colors, vector<bool> &recStack){
  if(colors[v] == white){
    colors[v] = grey;
    recStack[v] = true;
    //Recurse for all adjancent nodes
    for(unsigned int u : adj_nodes[v]){
      if(colors[u]==white && cycleRec(u, colors, recStack))
        return true;
      else if(recStack[u])
        return true;
    }
  }
  recStack[v] = false;
  return false;
}
//Since we are only using this for determining DAG, we know this will only be called for directed graphs.
template<typename T>
bool Graph<T>::isCyclic() {
  //Generate color map & recursion stack
  map<unsigned int, color_t> colors;
  vector<bool> recStack;
  for(unsigned int i=0; i<nodes.size(); i++){
    colors.insert(pair<unsigned int, color_t>(i, white));
    recStack.push_back(false);
  }
  //For every node we call our recursion on the node using its key.
  for(unsigned int i=0; i<nodes.size(); i++){
    if(cycleRec(i, colors, recStack))
      return true;
  }
  return false;
}

template <typename T>
bool Graph<T>::isDAG() {
  if(!digraph)
    return false;
  return !isCyclic();
}

template <typename T>
void Graph<T>::DFS(T s){
  stack<unsigned int> q;
  int sk=0;
  bool sk_found = false;
  //Generate color map && find node key
  map<unsigned int, color_t> colors;
  for(unsigned int i=0; i<nodes.size(); i++){
    colors.insert(pair<unsigned int, color_t>(i, white));
    if(comp(s, nodes[i])){
      sk_found = true;
      sk = i;
    }
  }

  //Node doesn't exist in our graph.
  if(!sk_found)
    return;
    //Mark it visited
  colors[sk] = grey;
  //Push ti to the top of our stack.
  q.push(sk);
  while(q.size()!=0){
    //Retreive top of stack & pop it
    unsigned int v = q.top();
    cout << nodes[v] << " ";
    q.pop();

    //Visit adjacency list of v
    for(unsigned int u : adj_nodes[v]){
      if(colors[u] == white){
        //If we haven't visited, we'll push it onto our stack and mark it visited.
        colors[u] = grey;
        q.push(u);
      }
    }
  }
  cout << endl;
}

//Iterative topological sort.
template <typename T>
void Graph<T>::topSort(){
  if(!isDAG()){
    cout << "Can't perform topological sort on non DAG." << endl;
    return;
  }
  if(nodes.size()==0){
    cout << "Can't top sort an empty list." << endl;
    return;
  }
  //Generate color map && find node key
  map<unsigned int, color_t> colors;
  for(unsigned int i=0; i<nodes.size(); i++){
    colors.insert(pair<unsigned int, color_t>(i, white));
  }
  stack<unsigned int> sorted_stack; //Tracks our sorted stack.
  //for every node
  for(unsigned int v=0; v<nodes.size(); v++){
    //If it's not visited
    if(colors[v]==white){
      colors[v] = grey;
      //Mark it visited then push all of its neighbors onto the stack if they're not visited and mark them visited.
      for(unsigned int u : adj_nodes[v]){
          if(colors[u]==white){
            colors[u] = grey;
            sorted_stack.push(u);
          }
      }
      //Then push this onto the stack.
      sorted_stack.push(v);
    }
  }
  // Prints topological sort from the top of the stack down.
  while(sorted_stack.size()>0){
    cout << nodes[sorted_stack.top()] << " ";
    sorted_stack.pop();
  }
  cout << endl;
}

//Recursive topolgoical sort that uses our second recursive DFS function that tracks time of visit.
// It's pretty much the same since our depart vector is acting like our stack because we're stuffing things under the same criteria
// which is visting a node, pushing its neighbors into the time stack, then itself. Then visiting a node that hasn't been visited and doing it again.
//Returns a vector containing the departure times. Helps with determining fully connected components
template <typename T>
vector<unsigned int> Graph<T>::topSort2(bool p, bool dag){
  //For this one I made this optional since it's useful for finding connected components
  if(dag){
    if(!isDAG()){
      cout << "Can't perform topological sort on non DAG." << endl;
      return vector<unsigned int>();
    }
  }
  if(nodes.size()==0){
    cout << "Can't top sort an empty list." << endl;
    return vector<unsigned int>();
  }
  //Generate color map && find node key
  map<unsigned int, color_t> colors;
  for(unsigned int i=0; i<nodes.size(); i++){
    colors.insert(pair<unsigned int, color_t>(i, white));
  }
  //Our pseudo stack.
  vector<unsigned int> depart(nodes.size(), 0);
  int time = 0;
  //For every node that isn't visited, run DFS on it for the nodes that aren't visited and mark the time they're visited.
  for(unsigned int i=0; i<nodes.size(); i++){
    if(colors[i]==white)
      rDFS(i, colors, depart, time);
  }
  //If we're using this as a print function, print it out(again, made this variable so I can use it in printing fully connected components)
  if(p){
    for(int i=depart.size()-1; i>=0; i--){
      cout << depart[i] << " ";
    }
    cout << endl;
  }
  return depart;
}
template <typename T>
unsigned int Graph<T>::getVertexCount() const{
  return vertex_count;
}
template <typename T>
unsigned int Graph<T>::getEdgeCount() const{
  return edge_count;
}

template <typename T>
void Graph<T>::printFullyConnectedComponents(){
  //Generate color map
  map<unsigned int, color_t> colors;
  for(unsigned int i=0; i<nodes.size(); i++){
    colors.insert(pair<unsigned int, color_t>(i, white));
  }
  //Find the finish times of every node
  vector<unsigned int> finish_times = topSort2(false, false);

  //generate the transpose.
  Graph<T> trans(*this);
  trans.transpose();
  //Starting from the back, I run DFS on each component since running it from behind shows connected components.
  //And tracking teh color map prevents overlap.
  while(finish_times.size()>0){
    int v = finish_times.back();
    finish_times.pop_back();
    if(colors[v] == white){
      trans.DFSRec(v, colors);
      cout << endl;
    }
  }
}

//Random vertex cover function
template <typename T>
void Graph<T>::printRVertexColor(){
  if(edges.size()==0){
    cout << "Not sure if 0 edges means they're all in the set or they're all excluded. I read that it meant they're all excluded, but I have my reservations about that." << endl;
    return;
  }
  //Tracking our cover using boolean array, could do it just as easily with a color map/vector
  bool cover[nodes.size()];
  //random seed
  srand(time(nullptr));
  //Setting covered vertices to be false.
  for(unsigned int i=0; i<nodes.size(); i++){
    cover[i] = false;
  }
  //Generating a copy of our edge list.
  vector<pair<unsigned int, unsigned int>> ee = edges;
  vector<pair<unsigned int, unsigned int>>::iterator it;
  unsigned int first;
  unsigned int second;

  while(ee.size()>0){
    // I adapted the iterative one and got away with minimal changes by just setting e to be random here.
    int e = rand() % ee.size();
    first = ee[e].first;
    second = ee[e].second;
    //Mark it covered.
    cover[first] = true;
    cover[second] = true;
    //Instead of popping i have to remove the element from possibly the middle
    //This takes the beginning(ee[0]) and adds the random number to it. So if we get e = 1, it'd be ee[1], accessing the second element.
    ee.erase(ee.begin() + e);
    //For each edge left in the vector, if it shares a vertex we remove it.
    for(it = ee.begin(); it!=ee.end(); ){
      if(it->first == first || it->second==second || it->first==second || it->second==first){
        it = ee.erase(it);
      }
      else
        it++;
    }
  }
  //Print it out based on the cover array.
  for(unsigned int i=0; i<nodes.size(); i++){
    if(cover[i])
      cout << nodes[i] << " ";
  }
  cout << endl;
}
//Uncovered deterministic vertex color
template <typename T>
void Graph<T>::printUVertexColor(){
  if(edges.size()==0){
    cout << "Not sure if 0 edges means they're all in the set or they're all excluded. I read that it meant they're all excluded, but I have my reservations about that." << endl;
    return;
  }
  //Generate covered boolean array. Could also be a color array.
  bool cover[nodes.size()];
  for(unsigned int i=0; i<nodes.size(); i++){
    cover[i] = false;
  }
  //Copy of edge list.
  vector<pair<unsigned int, unsigned int>> ee = edges;
  vector<pair<unsigned int, unsigned int>>::iterator it;
  unsigned int first;
  unsigned int second;

  while(ee.size()>0){
    //For every edge, starting with the back
    first = ee.back().first;
    second = ee.back().second;
    //Mark it visited and remove it
    cover[first] = true;
    cover[second] = true;
    ee.pop_back();
    //Visit all remaining edges to determine if it shares a vertex, if it does remove it.
    for(it = ee.begin(); it!=ee.end(); ){
      if(it->first == first || it->second==second || it->first==second || it->second==first){
        it = ee.erase(it);
      }
      else
        it++;
    }
  }
  //Print.
  for(unsigned int i=0; i<nodes.size(); i++){
    if(cover[i])
      cout << nodes[i] << " ";
  }
  cout << endl;
}
