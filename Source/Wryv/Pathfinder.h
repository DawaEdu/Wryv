#pragma once

#include <assert.h>
#include <float.h>
#include <vector>
#include <set>
#include <map>
#include <deque>
#include "GlobalFunctions.h"
using namespace std;

inline int index( int row, int col, int cols )
{
  return row*cols + col;
}

// Pathfinder's coordinates for building graph
struct Coord
{
  static int Rows, Cols;
  int row, col ;

  Coord() { row = col = 0 ; }
  Coord( int irow, int icol ) { row = irow ;  col = icol ; }
  Coord( int index ) { row = index / Cols;  col = index % Cols; }
  Coord( const Coord& other ) { row = other.row ; col = other.col ; }
  Coord( float irow, float icol )
  {
    row = (int)irow ;
    col = (int)icol ;
  }
  static int index( int row, int col ) { return row*Cols + col; }
  inline int index() const { return index( row, col ); }
  
  Coord toRight(){ return Coord( row, col+1 ); }
  Coord toLeft(){ return Coord( row, col-1 ); }

  // Counting with +row going up
  Coord Above(){ return Coord( row+1, col ); }
  Coord toUpperRight(){ return Coord( row+1, col+1 ); }
  Coord toUpperLeft(){ return Coord( row+1, col-1 ); }
    
  Coord Below(){ return Coord( row-1, col ); }
  Coord toLowerRight(){ return Coord( row-1, col+1 ); }
  Coord toLowerLeft(){ return Coord( row-1, col-1 ); }
  bool isValid() const { return 0 <= row && row < Rows   &&   0 <= col && col < Cols; }

  bool operator==( const Coord& coord ) const { return row == coord.row && col == coord.col; }
  bool operator<( const Coord& other ) const
  {
    // we'll use the magnitude, considering them like vectors.
    if( row < other.row )
      return true;
    else if( row == other.row )
      return col < other.col;
    return false;
  }

  // Finds manhattan distance to another Coord
  float manhattanTo( const Coord & other ) const 
  {
    float x = fabsf( (float)(other.col - col) ) ;
    float y = fabsf( (float)(other.row - row) ) ;
    return x + y ;
  }

  // euclidean distance to another node.
  // we'll use this one for the heuristic function.
  float euclideanTo( const Coord & other ) const
  {
    float x = (float)(other.col - col);
    float y = (float)(other.row - row);
    return sqrtf( x*x + y*y ) ;
  }
  
  vector<Coord> getNeighbours(){
    vector<Coord> n;
    n.push_back( toRight() );
    n.push_back( toLeft() );
    n.push_back( Above() );
    n.push_back( toUpperRight() );
    n.push_back( toUpperLeft() );
    n.push_back( Below() );
    n.push_back( toLowerRight() );
    n.push_back( toLowerLeft() );
    return n;
  }
} ;

struct GraphNode ;

struct Edge
{
  GraphNode *src, *dst ;
  float cost ;

  Edge()
  {
    src = dst = 0;
    cost = 0.0f;
  }

  Edge( GraphNode *sourceNode, GraphNode *destinationNode, float theCost )
  {
    src = sourceNode ;
    dst = destinationNode ;
    cost = theCost ;
  }

  bool connects( GraphNode* isrc, GraphNode* idst ) const
  {
    return (src==isrc && dst==idst) || (src==idst && dst==isrc);
  }
  
  bool connects( GraphNode* node ) const
  {
    return src==node || dst==node;
  }

  GraphNode* other( GraphNode* node ) {
    if( src == node )  return dst;
    else return src;
  }
} ;



struct GraphNode
{
  vector<Edge *> edges ;

  // Space point this node represents.
  Coord index;

  // A character representing the terrain at this node
  int terrain;
  
  // This is an indexed value again, and it is set once a node has been
  // visited and a cost to arrive here has been determined.
  // Because A* only "REACHES" each node ONCE and ONLY ONCE,
  // this value gets set once and only once when you first arrive at the node
  float costToGetHere ;

  FVector point;

  GraphNode( int iindex, int iterrain )
  {
    index = Coord( iindex );
    terrain = iterrain;
    costToGetHere = 0;
    point = FVector( index.row, index.col, 0.f );
  }

  // priority.
  bool operator<( const GraphNode& other )
  {
    return costToGetHere < other.costToGetHere ;
  }

  Edge* alreadyConnectedTo( GraphNode *o )
  {
    for( int i = 0; i < edges.size(); i++ )
      if( edges[i]->dst == o || edges[i]->src == o )
        return edges[i];
    return 0;
  }

  // Clear all outbound & inbound connections to this node
  void clearConnections()
  {
    for( int i = edges.size() - 1; i >= 0; i-- )
    {
      if( edges[i]->src == this )
        // walk dst and clear any connecting edges to that
        edges[i]->dst->clearConnectionsTo( this );
      else if( edges[i]->dst == this )
        edges[i]->src->clearConnectionsTo( this );
    }
    edges.clear();
  }

  void clearConnectionsTo( GraphNode* o )
  {
    for( int i = edges.size() - 1; i >= 0; i-- )
      if( edges[i]->connects( o ) )
        edges.erase( edges.begin() + i );
  }
  
  // This is used when backtracing the lowest cost path from the
  // endPos to the startPos.
  GraphNode* getLowestCostAdjacentNodeInSet( set<GraphNode*> list )
  {
    GraphNode* lowestCostNeighbour = 0;
    float lowestCostSoFar = FLT_MAX;
    
    for( int i = 0; i < edges.size(); i++ )
    {
      Edge *edge = edges[i];
      GraphNode *node = edge->other( this );
      // only valid if n is in the closedList
      if( list.find( node ) != list.end() &&
          node->costToGetHere < lowestCostSoFar )
      {
        lowestCostSoFar = node->costToGetHere;
        lowestCostNeighbour = node;
      }
    }

    return lowestCostNeighbour;
  }

  ~GraphNode()
  {
    // !! DO NOT DELETE THE EDGES.
    // They are simply mirrored copies
    // of the edges in the global list.
    // the main game object will take care
    // of the global list.
  }
} ;

enum Terrain
{
  Impassible,
  Passible
};

class Pathfinder
{
public:
  GraphNode *startNode, *endNode;
  vector<GraphNode *> nodes ;
  vector<Edge *> edges ;
  
private:
  void init( int rows, int cols )
  {
    Coord::Rows = rows ;
    Coord::Cols = cols ;

    nodes.resize( rows*cols );
    for( int i = 0; i < nodes.size(); i++ )
      nodes[i] = new GraphNode( i, Passible );
    connectGraph();

    startNode = *nodes.begin();
    endNode = *(--nodes.end());
  }
  
public:
  Pathfinder() { init( 20, 20 ) ; }
  Pathfinder( int rows, int cols ) { init( rows, cols ) ; }

  void connectGraph()
  {
    for( int i = 0 ; i < nodes.size() ; i++ )
      updateGraphConnections( Coord( i ) );
  }

  void updateGraphConnections( Coord loc )
  {
    GraphNode *node = nodes[ loc.index() ] ;

    // Check in 8 directions what nodes this node can reach and at what cost.
    vector<Coord> coords = node->index.getNeighbours();
    for( int i = 0; i < coords.size(); i++ )
      checkNode( node, coords[i] );
  }

  void connect( GraphNode *src, GraphNode *dst )
  {
    Edge* edge = src->alreadyConnectedTo( dst );
    if( !edge ) edge = dst->alreadyConnectedTo( src );

    // couldn't find an edge connecting src->dst or dst->src,
    // now searching group of "loose" edges
    for( int i = 0; i < edges.size() && !edge; i++ )
    {
      if( edges[i]->connects( src, dst ) ) {
        edge = edges[i];
        // restore the edge by pushing into both collections
        src->edges.push_back( edge );
        dst->edges.push_back( edge );
      }
    }

    if( edge )
    {
      // Update the edge weights of the existing connections
      edge->cost = 1.f;
    }
    else
    {
      // need a new edge.
      // Let edge cost 1/2 sum of src cost + dst cost
      edge = new Edge( src, dst, 1.f ) ;
      //LOG(  "Edge connecting %d<=>%d created",
      //  src->index.index(), dst->index.index() );
      
      // Save the same edge into both src and dst.
      src->edges.push_back( edge );
      if( !dst->alreadyConnectedTo( src ) )
        dst->edges.push_back( edge );
      
      edges.push_back( edge ) ;
    }
  }

  // Check if this node should connect to the node at 'coord', and if so,
  // create an appropriately costed EDGE for the connection.
  void checkNode( GraphNode *src, const Coord &coord )
  {
    // To check this coordinate, both the row and col must be IN BOUNDS.
    if( !coord.isValid() )  return; // out of bounds.
    GraphNode *dst = nodes[ coord.index() ] ;

    // Dst is an impassible node and no connection can be formed to DST
    // from SRC in this direction (coord)
    if( dst->terrain == Impassible )
      return;

    // src && dst are both passible, form a connection to dst
    if( src->terrain == Passible )
    {
      connect( src, dst );
    }
    else // src is an impassible.
    {
      // Its INFINITE, no edge to <<dir>> then.
      // if there was an edge, remove the edge
      //src->removeConnectionTo( dst );
      // ALL inbounds closed if impassible
      src->clearConnections();
    }
  }

  void destroyGraph()
  {
    // destroy the old graph nodes AND edges.
    for( int i = 0 ; i < nodes.size() ; i++ ) delete nodes[ i ] ;
    nodes.clear();

    for( int i = 0 ; i < edges.size() ; i++ ) delete edges[ i ] ;
    edges.clear();
  }

  // PULL means get it out and remove it 
  // from the list and return it as well.
  GraphNode* pullLowestCostNode(set<GraphNode*> &list)
  {
    // Find the lowest cost node to walk along from here
    set<GraphNode*>::iterator lowestCostNodeIter = list.begin() ;
    float lowestCostSoFar = (*lowestCostNodeIter)->costToGetHere;
    
    for( set<GraphNode *>::iterator iter = list.begin() ;
         iter != list.end(); ++iter )
    {
      GraphNode* node = *iter ;
      if( node->costToGetHere < lowestCostSoFar )
      {
        lowestCostSoFar = node->costToGetHere;
        lowestCostNodeIter = iter;
      }
    }

    GraphNode *lowestCostNode = *lowestCostNodeIter ;
    list.erase( lowestCostNodeIter ) ; // remove it from the LIST
    printf( "Exploring from node (%d, %d) with cost %.3f\n",
      lowestCostNode->index.row, lowestCostNode->index.col,
      lowestCostNode->costToGetHere ) ;
    return lowestCostNode ;
  }

  // The node near pos, that at least is in the direction dir.
  GraphNode* findNear( FVector pos, FVector dir )
  {
    dir.Normalize();
    map< float, GraphNode* > dists;
    /// linear search through the nodes and find the one nearest pos
    for( int i = 0; i < nodes.size(); i++ )
    {
      if( !nodes[i] || nodes[i]->terrain == Impassible )  continue;
      // Dot product less than or equal to DotAngle to be within cone
      float dist = FVector::Dist( pos, nodes[i]->point );
      dists[ dist ] = nodes[i];
    }

    if( !dists.size() )
    {
      error( "There were no nodes in dists" );
      return 0;
    }

    // Find the closest 4 nodes with prioritize angle on top of prox.
    // ie choose the point with maximal dot product with dir.
    GraphNode *node = 0;
    float largestDot = -1.f;
    int count = 0;
    for( pair< const float, GraphNode* > p : dists )
    {
      FVector towards = p.second->point - pos;
      towards.Normalize();
      float dot = FVector::DotProduct( dir, towards );
      if( dot >= largestDot )
      {
        node = p.second;
        largestDot = dot;
      }

      count++;
      if( count >= 4 ) break;
    }

    if( !node )
    {
      error( FS( "Couldn't find node near %f %f %f", pos.X, pos.Y, pos.Z ) ) ;
    }
    // shortest distance is front
    return node;
  }

  // A* algorithm.
  // pseudocode:
  // while the open list is not empty
  // {
  //   current_node = node from open list ___with lowest cost___ FROM START NODE
  //   if current_node equals goal_node then
  //     END ;
  //   else
  //     move current node to the closed list
  //     examine each node adjacent to the current node
  //     foreach adjacent node
  //       if it isn't in the open list and it isn't in the closed list
  //         move it to the open list and CALCULATE ITS COST
  // }
  vector<FVector> findPath( FVector src, FVector dst )
  {
    vector<FVector> finalPath;
    set<GraphNode *> openList, closedList;
    
    // Find the node nearest src
    FVector dir = dst - src;
    startNode = findNear( src, dir );
    endNode = findNear( dst, dir );
    if( !startNode || !endNode )  return finalPath;
    startNode->costToGetHere = FVector::Dist( src, startNode->point );
    openList.insert( startNode ) ;

    // Last pt to add in is DST itself
    finalPath.insert( finalPath.begin(), dst );
    
    while( !openList.empty() )
    {
      // The current_node is the lowest cost node in the OpenList.
      // really this should be the "node with lowest cost to reach" from the open list.
      GraphNode *current_node = pullLowestCostNode(openList);
      if( current_node == endNode )
      {
        // Write the final path by backtracking along from the endPos
        // back to the startPos, lowest cost
        current_node = endNode;
        
        while( current_node != startNode )
        {
          // Walk towards the lowest cost neighbour,
          // pushing each node into the finalPath as we go
          finalPath.insert( finalPath.begin(), current_node->point );
          current_node = current_node->getLowestCostAdjacentNodeInSet( closedList );
        }

        finalPath.insert( finalPath.begin(), startNode->point );
        openList.clear(); // signals to exit
        
        return finalPath; // just in case
      }
      else
      {
        // move the current_node to the closedList, which means
        // it is part of the solution
        closedList.insert( current_node ) ;

        // now working from the current_node, we need to examine ALL adjacent nodes
        // to this current_node.
        for( int i = 0 ; i < current_node->edges.size() ; i++ )
        {
          Edge *edge = current_node->edges[ i ];
          GraphNode *adjacentNode;
          if( edge->src == current_node )  adjacentNode = edge->dst;
          else if( edge->dst == current_node )  adjacentNode = edge->src;

          // if (the adjacent node) isn't  in open or closed list,
          // move it to the open list and compute cost.

          // If in either open or closed list, it has already been EXPLORED
          if( openList.find( adjacentNode ) == openList.end() &&
              closedList.find( adjacentNode ) == closedList.end() )
          {
            // then its not in the open or closed list
            // so move it to the open list and compute its cost
            // to get there.
            openList.insert( adjacentNode ) ;
              
            // Compute the value of the HEURISTIC function
            // (which is just the euclidean distance to
            // the goal node)
            float heuristicFnValue = FVector::Dist( adjacentNode->point, endNode->point ) ;//adjacentNode->index.euclideanTo( endNode->index ) ;
            //float heuristicFnValue = adjacentNode->coord.manhattanTo( endNode->index ) ;
            // The heuristic function helps to "steer" the
            // exploration pattern and drive it to
            // the goal node
              
            adjacentNode->costToGetHere = current_node->costToGetHere   +
              edge->cost   +   heuristicFnValue ;
          }
        }
      }
    }

    return finalPath;
  }

  // Ensure the graph
  ~Pathfinder()
  {
    destroyGraph();
  }
} ;


