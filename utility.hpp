#ifndef UTILITY_CPP
#define UTILITY_CPP

#include "astar_search.h"
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/grid_graph.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <ctime>
#include <iostream>
#include "visualizer.h"


#include <type_traits>
#include <typeinfo>
#include <memory>
#include <string>
#include <cstdlib>

const int cwConstant = 5;

enum OverrideFlags
{
    OF_RIVER_MARSH = 0x10,
    OF_INLAND = 0x20,
    OF_WATER_BASIN = 0x40
};

// Some constants
enum {
    IMAGE_DIM = 2048, // Width and height of the elevation and overrides image
    
    ROVER_X = 159,
    ROVER_Y = 1520,
    BACHELOR_X = 1303,
    BACHELOR_Y = 85,
    WEDDING_X = 1577,
    WEDDING_Y = 1294
};


boost::mt19937 random_generator;

// Distance traveled in the maze
typedef double distance;

#define GRID_RANK 2
typedef boost::grid_graph<GRID_RANK> grid;
typedef boost::graph_traits<grid>::vertex_descriptor vertex_descriptor;
typedef boost::graph_traits<grid>::vertices_size_type vertices_size_type;

// A hash function for vertices.
struct vertex_hash:std::unary_function<vertex_descriptor, std::size_t> {
  std::size_t operator()(vertex_descriptor const& u) const {
    std::size_t seed = 0;
    boost::hash_combine(seed, u[0]);
    boost::hash_combine(seed, u[1]);
    return seed;
  }
};

typedef boost::unordered_set<vertex_descriptor, vertex_hash> vertex_set;
typedef boost::vertex_subset_complement_filter<grid, vertex_set>::type
        filtered_grid;

// A searchable maze
//
// The maze is grid of locations which can either be empty or contain a
// barrier.  You can move to an adjacent location in the grid by going up,
// down, left and right.  Moving onto a barrier is not allowed.  The maze can
// be solved by finding a path from the lower-left-hand corner to the
// upper-right-hand corner.  If no open path exists between these two
// locations, the maze is unsolvable.
//
// The maze is implemented as a filtered grid graph where locations are
// vertices.  Barrier vertices are filtered out of the graph.
//
// A-star search is used to find a path through the maze. Each edge has a
// weight of one, so the total path length is equal to the number of edges
// traversed.
class maze {
public:
  friend std::ostream& operator<<(std::ostream&, const maze&);
  friend maze random_maze(std::size_t, std::size_t);

  maze():m_grid(create_grid(0, 0)),m_barrier_grid(create_barrier_grid()) {};
  maze(std::size_t x, std::size_t y):m_grid(create_grid(x, y)),
       m_barrier_grid(create_barrier_grid()) {};

  // The length of the maze along the specified dimension.
  vertices_size_type length(std::size_t d) const {return m_grid.length(d);}

  bool has_barrier(vertex_descriptor u) const {
    return m_barriers.find(u) != m_barriers.end();
  }

 

  bool solve(vertex_descriptor source, vertex_descriptor goal);
  bool solved() const {return !m_solution.empty();}
  bool solution_contains(vertex_descriptor u) const {
    return m_solution.find(u) != m_solution.end();
  }

  // Create the underlying rank-2 grid with the specified dimensions.
  grid create_grid(std::size_t x, std::size_t y) {
    boost::array<std::size_t, GRID_RANK> lengths = { {x, y} };
    return grid(lengths);
  }

  // Filter the barrier vertices out of the underlying grid.
  filtered_grid create_barrier_grid() {
    return boost::make_vertex_subset_complement_filter(m_grid, m_barriers);
  }

  // The grid underlying the maze
  grid m_grid;
  // The underlying maze grid with barrier vertices filtered out
  filtered_grid m_barrier_grid;
  // The barriers in the maze
  vertex_set m_barriers;
  // The vertices on a solution path through the maze
  vertex_set m_solution;
  // The length of the solution path
  distance m_solution_length;
};


// Euclidean heuristic for a grid
//
// This calculates the Euclidean distance between a vertex and a goal
// vertex.
class euclidean_heuristic:
      public boost::astar_heuristic<filtered_grid, double>
{
public:
  euclidean_heuristic(vertex_descriptor goal):m_goal(goal) {};

  double operator()(vertex_descriptor v) {
    return sqrt(pow(double(m_goal[0] - v[0]), 2) + pow(double(m_goal[1] - v[1]), 2));
  }

private:
  vertex_descriptor m_goal;
};

class jim_heuristic:
public boost::astar_heuristic<filtered_grid, double>
{
public:
jim_heuristic(vertex_descriptor goal):m_goal(goal) {};

double operator()(vertex_descriptor v) {
return 10*(double(abs(m_goal[0] - v[0]) + double(abs(m_goal[1] - v[1]))));
}

private:
vertex_descriptor m_goal;
};


// Exception thrown when the goal vertex is found
struct found_goal {};

// Visitor that terminates when we find the goal vertex
struct astar_goal_visitor:public boost::default_astar_visitor {
  astar_goal_visitor(vertex_descriptor goal):m_goal(goal) {};

  void examine_vertex(vertex_descriptor u, const filtered_grid&) {
    if (u == m_goal)
      throw found_goal();
  }

private:
  vertex_descriptor m_goal;
};

// Solve the maze using A-star search.  Return true if a solution was found.
bool maze::solve(vertex_descriptor source, vertex_descriptor goal) {
  boost::static_property_map<distance> weight(1);
  // The predecessor map is a vertex-to-vertex mapping.
  typedef boost::unordered_map<vertex_descriptor,
                               vertex_descriptor,
                               vertex_hash> pred_map;
  pred_map predecessor;
  boost::associative_property_map<pred_map> pred_pmap(predecessor);
  // The distance map is a vertex-to-distance mapping.
  typedef boost::unordered_map<vertex_descriptor,
                               distance,
                               vertex_hash> dist_map;
  dist_map distance;
  boost::associative_property_map<dist_map> dist_pmap(distance);

  jim_heuristic heuristic(goal);
  astar_goal_visitor visitor(goal);

  std::cout << "here! before try in solve()" << std::endl;
  try {
    astar_search(m_barrier_grid, source, heuristic,
                 boost::weight_map(weight).
                 predecessor_map(pred_pmap).
                 distance_map(dist_pmap).
                 visitor(visitor) );
    std::cout << "here! after try in solve()" << std::endl;
  } catch(found_goal fg) {
    // Walk backwards from the goal through the predecessor chain adding
    // vertices to the solution path.
    for (vertex_descriptor u = goal; u != source; u = predecessor[u])
      m_solution.insert(u);
    m_solution.insert(source);
    m_solution_length = distance[goal];
    return true;
  }

  return false;
}


// Return a random integer in the interval [a, b].
std::size_t random_int(std::size_t a, std::size_t b) {
  if (b < a)
    b = a;
  boost::uniform_int<> dist(a, b);
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> >
  generate(random_generator, dist);
  return generate();
}

double stepTime(const vertex_descriptor& source, const vertex_descriptor& target, const std::vector<uint8_t>& elevation)
{
  double stepTime = 0;
  bool diag = (abs(source[0]-target[0]) == abs(source[1]-target[1])) ? 1 : 0;
  auto sourceElevation = static_cast<int>(elevation[source[0]+source[1]*IMAGE_DIM]);
  auto targetElevation = static_cast<int>(elevation[target[0]+target[1]*IMAGE_DIM]);

  //check for water or flat
  if(sourceElevation == 0 || targetElevation == 0)
  {
    std::cout<< "ERROR! elevation of path is 0";
    return 0;
  }
  
  //std::cout << "source elevation: " << sourceElevation << std::endl;
  int delta = targetElevation - sourceElevation;

  if(!diag)
  {
    if(delta == 0)
    {
      stepTime += 1;
    }
    else if(delta > 0 )
      {
        stepTime +=  sqrt(1 + 0.003937*pow(delta,2)) + cwConstant*0.0627455*delta;
      }
    else
    { 
        stepTime += sqrt(1 + 0.003937*pow(delta,2)) - cwConstant*0.0627455*delta;
    }
    
  }
  else{
    if(delta == 0)
    {
      stepTime += 1.414;
    }
    else if(delta > 0 )
      {
        stepTime +=  sqrt(2 + 0.003937*pow(delta,2)) + cwConstant*0.0627455*delta;
      }
    else
    { 
        stepTime += sqrt(2 + 0.003937*pow(delta,2)) - cwConstant*0.0627455*delta;
    }
  }
  return stepTime;
}
// Generate a maze with a random assignment of barriers.
maze make_maze(std::size_t x, std::size_t y, const std::vector<uint8_t>& overrides, const std::vector<uint8_t>& elevation) {
  maze m(IMAGE_DIM, IMAGE_DIM);
  auto Obegin = overrides.begin();
  vertex_descriptor u;
  int count = 0;
  auto Ebegin = elevation.begin();
  for(auto i = overrides.begin(); i != overrides.end(); ++i)
  {
      if((*i & (OF_WATER_BASIN | OF_RIVER_MARSH)) || (elevation[i-Obegin] == 0))
      {
          count++;
          auto xBarr = (i - Obegin)%IMAGE_DIM;
          auto yBarr = ((i - Obegin) - xBarr)/IMAGE_DIM;

          //std::cout << "barriers are at " << xBarr <<", "<<yBarr <<std::endl;

          u = vertex((i-Obegin), m.m_grid);
          m.m_barriers.insert(u);
      }
    }
    std::cout << "count" << count << std::endl;
  return m;
}
#endif