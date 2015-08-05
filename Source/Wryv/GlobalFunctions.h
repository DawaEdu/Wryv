#pragma once

#include "Engine.h"

#include <vector>
#include <set>
#include <map>

using namespace std;

// CALL: LOG( "Format string %d", intValue );
#define LOG(x, ...) UE_LOG( LogTemp, Warning, TEXT( x ), __VA_ARGS__ )

template <typename T> T* removeElement( vector<T*>& v, T* elt )
{
  for( int i = v.size()-1 ; i >= 0; i-- )
  {
    if( v[i] == elt )
    {
      v.erase( v.begin() + i );
      return elt;
    }
  }
  
  return 0;  // The element wasn't found in the list
}

template <typename T> T* removeIndex( vector<T*>& v, int index )
{
  if( index < 0 || index >= v.size() )
  {
    LOG( "vector::removeIndex(%d) OOB", index );
    return 0;
  }

  // get the element, remove from vector
  T* elt = v[index];
  v.erase( v.begin() + index );
  return elt;
}

template <typename T> bool removeIndex( vector<T>& v, int index )
{
  if( index < 0 || index >= v.size() )
  {
    LOG( "vector::removeIndex(%d) OOB", index );
    return 0;
  }

  // get the element, remove from vector
  v.erase( v.begin() + index );
  return 1;
}

template <typename T> bool removeElement( vector<T>& v, T& elt )
{
  for( int i = v.size()-1 ; i >= 0; i-- )
  {
    if( v[i] == elt )
    {
      v.erase( v.begin() + i );
      return 1;
    }
  }

  return 0;
}

template <typename T> bool remove( set<T>& s, T& elt )
{
  set<T>::iterator it = s.find( elt );
  if( it != s.end() )
  {
    s.erase( it );
    return 1;
  }
  return 0;
}

template <typename T> inline bool in( set<T>& s, T& elt )
{
  return s.find( elt ) != s.end();
}

/// Removes an element t from a vector v
template <typename T> bool pop_front( vector<T>& v )
{
  if( v.size() )
  {
    v.erase( v.begin() );
    return 1;
  }

  return 0;
}

/// Removes an element t from a vector v
template <typename T> bool pop_back( vector<T>& v, int eltNumber )
{
  if( eltNumber < v.size() )
  {
    // can erase the elt
    v.erase( v.begin() + eltNumber );
    return 1;
  }

  return 0;
}

inline FVector Rand()
{
  return FVector( FMath::FRand(), FMath::FRand(), FMath::FRand() );
}

inline FVector Rand( FVector min, FVector max )
{
  return min + FMath::FRand()*( max - min );
}

// Exclusive rand()
inline int randInt( int min, int max )
{
  int range = max-min;
  if( range )
    return min + FMath::Rand()%(max-min);
  else
    return min;
}

inline float randFloat( int min, int max )
{
  return min + FMath::FRand()*( max - min );
}

template <typename T> static T* GetComponentByType( AActor* a )
{
  TArray<UActorComponent*> comps = a->GetComponents();
  for( int i = 0; i < comps.Num(); i++ )
    if( T* s = Cast<T>( comps[i] ) )
      return s;
  return 0;
}

template <typename T> static vector<T*> GetComponentsByType( AActor* a )
{
  TArray<UActorComponent*> comps = a->GetComponents();
  //LOG( "# components %d", comps.Num() );
  vector<T*> coll;
  for( int i = 0; i < comps.Num(); i++ )
    if( T* s = Cast<T>( comps[i] ) )
      coll.push_back( s );
  return coll;
}

template <typename T> static T* GetComponentByName( AActor* a, FString name )
{
  TArray<UActorComponent*> comps = a->GetComponents();
  for( int i = 0; i < comps.Num(); i++ )
    if( T* s = Cast<T>( comps[i] ) )
      if( s->GetName() == name )
        return s;
  return 0;
}

inline bool Intersects( FBox& box, FVector& pt )
{
  return box.Min.X < pt.X && pt.X < box.Max.X   &&
         box.Min.Y < pt.Y && pt.Y < box.Max.Y   &&
         box.Min.Z < pt.Z && pt.Z < box.Max.Z ;
}

inline void print( FBox& box )
{
  LOG( "%.2f %.2f %.2f,  %.2f %.2f %.2f",
    box.Min.X,box.Min.Y,box.Min.Z, box.Max.X,box.Max.Y,box.Max.Z );
}

struct Bezier
{
  FVector P0, P1, P2, P3;
  Bezier( const FVector& A, const FVector& B, const FVector& C, const FVector& D ) :
    P0( A ), P1( B ), P2( C ), P3( D ) { }
  FVector operator()( float t )
  {
    float l_t = 1.f - t;
    return P0*l_t*l_t*l_t + P1*l_t*l_t*t + P2*l_t*t*t + P3*t*t*t;
  }
};

struct Cubic
{
  FVector P0, P1, P2;
  Cubic( const FVector& A, const FVector& B, const FVector& C ) :
    P0( A ), P1( B ), P2( C ) { }
  FVector operator()( float t )
  {
    float l_t = 1.f - t;
    return P0*l_t*l_t + P1*l_t*t + P2*t*t;
  }
};

struct Linear
{
  FVector P0, P1;
  Linear( const FVector& A, const FVector& B ) :
    P0( A ), P1( B ) { }
  FVector operator()( float t )
  {
    float l_t = 1.f - t;
    return P0*l_t + P1*t;
  }
};




