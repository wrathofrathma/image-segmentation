# An image segmentation program using graph based algorithms.

Kind of broken mess right now since I ran out of time when converting from a naive approach using local radius around each pixel and adding edges between ones within a certain tolerance to a more mature approach of SLIC. By failing I learned why SLIC uses a 5 dimensional coordinate system in labxy space. It's an easy way to track the location of each cluster as the center changes.

I think the next approach will probably use a slightly different graph layout where the clusters are in the graph acting as the only edges between pixels. It will open up minimum path algorithms. 

### Technologies used
* SFML Graphics & Audio libraries for reading & writing the data.
* C++
* Boost graph library
