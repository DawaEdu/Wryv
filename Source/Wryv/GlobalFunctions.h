#pragma once

// Inlined functions
#include "Engine.h"

#include <vector>
#include <set>
#include <deque>
#include <functional>
#include <map>
#include <algorithm>
#include <stdarg.h>

using namespace std;

DECLARE_LOG_CATEGORY_EXTERN( K, Log, All );

#define FS(x,...) FString::Printf( TEXT( x ), __VA_ARGS__ )

// CALL: LOG( "Format string %d", intValue );
#define LOG(x, ...) UE_LOG( LogTemp, Warning, TEXT( x ), __VA_ARGS__ )

#define skip continue
#define self this

__forceinline void fatal( FString message )
{
  UE_LOG( K, Fatal, TEXT("%s"), *message );
}

__forceinline void error( FString message )
{
  UE_LOG( K, Error, TEXT("%s"), *message );
}

__forceinline void warning( FString message )
{
  UE_LOG( K, Warning, TEXT("%s"), *message );
}

__forceinline void info( FString message )
{
  UE_LOG( K, Display, TEXT("%s"), *message );
}

// Removes all occurrences of `elt` from `v`
template <typename T> int removeElement( vector<T*>& v, T* elt )
{
  int count = 0;
  for( int i = v.size()-1 ; i >= 0; i-- )
    if( v[i] == elt ) {
      v.erase( v.begin() + i );
      count++;
    }
  return count;
}

template <typename T> int removeElement( vector<T>& v, T& elt )
{
  int count = 0;
  for( int i = v.size()-1 ; i >= 0; i-- )
    if( v[i] == elt ) {
      v.erase( v.begin() + i );
      count++;
    }
  return count;
}

template <typename T> vector<T*>& operator-=( vector<T*>& v, T* elt )
{
  for( int i = v.size()-1 ; i >= 0; i-- )
    if( v[i] == elt )
      v.erase( v.begin() + i );
  return v;
}

template <typename T> vector<T>& operator-=( vector<T>& v, T& elt )
{
  for( int i = v.size()-1 ; i >= 0; i-- )
    if( v[i] == elt )
      v.erase( v.begin() + i );
  return v;
}

template <typename T> bool removeElement( set<T*>& s, T* elt )
{
  set<T*>::iterator it = s.find( elt );
  if( it != s.end() ) {
    s.erase( it );
    return 1;
  }
  return 0;
}

template <typename T> bool removeElement( set<T>& s, T& elt )
{
  set<T>::iterator it = s.find( elt );
  if( it != s.end() ) {
    s.erase( it );
    return 1;
  }
  return 0;
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

template <typename T> T removeIndex( vector<T>& v, int index )
{
  T elt;
  if( index < 0 || index >= v.size() )
  {
    LOG( "vector::removeIndex(%d) OOB", index );
  }

  // get the element, remove from vector
  elt = v[index];
  v.erase( v.begin() + index );
  return elt;
}

// Gives you first element, NULL if element doesn't exist
template <typename T> inline T first( set<T>& s )
{
  if( s.size() ) return *s.begin();
  else return 0;
}
template <typename T> inline T* first( set<T*>& s )
{
  if( s.size() ) return *s.begin();
  else return 0;
}
template <typename T> inline T first( vector<T>& v )
{
  if( v.size() ) return *v.begin();
  else return 0;
}
template <typename T> inline T* first( vector<T*>& v )
{
  if( v.size() ) return *v.begin();
  else return 0;
}

template <typename T> inline bool in( const set<T*>& s, const T* elt )
{
  return s.find( elt ) != s.end();
}

template <typename T> inline bool in( const set<T>& s, const T& elt )
{
  return s.find( elt ) != s.end();
}
template <typename T> inline bool in( const vector<T>& s, const T& elt )
{
  for( int i = 0; i < s.size(); i++ )
    if( s[i] == elt )
      return 1;
  return 0;
}
template <typename T> inline bool in( const vector<T*>& s, const T* elt )
{
  for( int i = 0; i < s.size(); i++ )
    if( s[i] == elt )
      return 1;
  return 0;
}

template <typename T> inline bool in( const deque<T>& d, const T& elt )
{
  return find( d.begin(), d.end(), elt ) != d.end();
}

template <typename T> inline int index( const vector<T>& s, const T& elt )
{
  for( int i = 0; i < s.size(); i++ )
    if( s[i] == elt )
      return i;
  return -1;
}
template <typename T> inline int index( const vector<T*>& s, const T* elt )
{
  for( int i = 0; i < s.size(); i++ )
    if( s[i] == elt )
      return i;
  return -1;
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

template <typename T> set<T*> Intersection( const set<T*>& A, const set<T*>& B )
{
  set<T*> intns;
  std::set_intersection( A.begin(), A.end(), B.begin(), B.end(), inserter( intns, intns.begin() ) );
  return intns;
}

template <typename T> vector<T*> Intersection( const vector<T*>& A, const vector<T*>& B )
{
  vector<T*> intns;
  std::set_intersection( A.begin(), A.end(), B.begin(), B.end(), back_inserter( intns ) );
  return intns;
}

// Adds B to A, removing duplicates
template <typename T> vector<T*>& operator+=( vector<T*>& A, const vector<T*>& B )
{
  for( T* b : B )
    if( !in( A, b ) )
      A.push_back( b ) ;
    else {
      LOG( "DUPLICATE: vector operator+=: don't keep: b was in A" );
    }
  return A;
}

template <typename T> vector<T>& operator+=( vector<T>& A, const T& b )
{
  if( !in( A, b ) ) {
    A.push_back( b ) ;
  }
  else {
    LOG( "DUPLICATE: vector operator+=: don't keep: b was in A" );
  }
  return A;
}

template <typename T> vector<T*>& operator+=( vector<T*>& A, const T* b )
{
  if( !in(A,b) ) {
    A.push_back( b )
  }
  else {
    LOG( "DUPLICATE: vector operator+=: don't keep: b was in A" );
  }
  return A;
}

template <typename T> vector<T*>& operator-=( vector<T*>& A, const vector<T*>& B )
{
  for( T* b : B )
    removeElement( A, b ) ;
  return A;
}

template <typename T> deque<T>& operator-=( deque<T>& A, const deque<T>& B )
{
  // At each A, check if in B. If in B, remove from A.
  for( deque<T>::iterator IterA = A.begin(); IterA != A.end(); )
  {
    if( in( B, *IterA ) )
      IterA = A.erase( IterA );
    else
      ++IterA;
  }
  return A;
}

template <typename T> set<T*>& operator+=( set<T*>& A, const set<T*>& B )
{
  for( T* b : B )
    A.insert( b );
  return A;
}

// Filters by removing elts from set 2 from set 1
template <typename T> set<T> operator|( set<T> src, const set<T>& forbidden )
{
  // Filter objects from the set
  for( set<T>::iterator it = src.begin(); it != src.end(); )
  {
    if( in( forbidden, *it ) ) // then must remove
    {
      set<T>::iterator it2 = it;
      ++it2;
      src.erase( it );
      it = it2;
    }
    else ++it;
  }

  return src;
}

template <typename T> vector<T> operator|( vector<T> src, const vector<T>& forbidden )
{
  // Filter objects from the set
  for( int i = src.size() - 1; i >= 0; i-- )
  {
    if( in( forbidden, src[i] ) ) // then must remove
    {
      src.erase( src.begin() + i );
    }
    else ++it;
  }

  return src;
}

// Filters by removing elts from set 2 from set 1
template <typename T> set<T*> operator|( set<T*> src, const set<T*>& forbidden )
{
  // Filter objects from the set
  for( set<T*>::iterator it = src.begin(); it != src.end(); )
  {
    if( in( forbidden, *it ) ) // then must remove
    {
      set<T*>::iterator it2 = it;
      ++it2;
      src.erase( it );
      it = it2;
    }
    else ++it;
  }
  return src;
}
template <typename T> vector<T*> operator|( vector<T*> src, const vector<T*>& forbidden )
{
  // Filter objects from the set
  for( int i = src.size() - 1; i >= 0; i-- )
  {
    if( in( forbidden, src[i] ) ) // then must remove
      src.erase( src.begin() + i );
  }
  return src;
}

template <typename T> set<T*> operator|( 
  set<T*> src, function< bool (T*) > doFilterFunction )
{
  // Filter objects from the set
  for( set<T*>::iterator it = src.begin(); it != src.end(); )
  {
    if( doFilterFunction( *it ) ) // then must remove
    {
      set<T*>::iterator it2 = it;
      ++it2;
      src.erase( it );
      it = it2;
    }
    else ++it;
  }
  return src;
}

template <typename T> vector<T*> operator|( 
  vector<T*> src, function< bool (T*) > doFilterFunction )
{
  // Filter objects from the set
  for( int i = src.size() - 1; i >= 0; i-- )
  {
    if( doFilterFunction( *it ) ) // then must remove
      src.erase( src.begin() + i );
  }
  return src;
}

template <typename T> set<T*>& operator|=( 
  set<T*>& src, function< bool (T*) > doFilterFunction )
{
  for( set<T*>::iterator it = src.begin(); it != src.end(); )
  {
    if( doFilterFunction( *it ) ) // then must remove
    {
      it = src.erase( it );       // erase prev & advance the iterator
    }
    else ++it;
  }

  return src;
}
template <typename T> vector<T*>& operator|=( 
  vector<T*>& src, function< bool (T*) > doFilterFunction )
{
  for( int i = src.size() - 1; i >= 0; i-- )
  {
    if( doFilterFunction( src[i] ) ) // then must remove
    {
      src.erase( src.begin() + i );
    }
  }

  return src;
}

template <typename T> set<T*>& operator-=( set<T*>& A, const set<T*>& B )
{
  for( T* t : B )
    removeElement( A, t );
  return A;
}

template <typename T> set<T> MakeSet( const vector<T>& A )
{
  set<T> B;
  for( T& t : A )
    B.insert( t );
  return B;
}
template <typename T> vector<T*> MakeVector( const set<T*>& A )
{
  vector<T*> B;
  for( T* t : A )
    B.push_back( t );
  return B;
}

template <typename T> vector<T> MakeVector( const set<T>& A )
{
  vector<T> B;
  for( T& t : A )
    B.push_back( t );
  return B;
}

inline FVector Rand()
{
  return FVector( FMath::FRand(), FMath::FRand(), FMath::FRand() );
}

inline FVector Rand( FVector min, FVector max )
{
  return min + FMath::FRand()*( max - min );
}

inline FVector Invert( FVector v )
{
  if( v.X ) v.X = 1.f/v.X; else { LOG( "FVector::InvertX() Error: Divide by 0" ); }
  if( v.Y ) v.Y = 1.f/v.Y; else { LOG( "FVector::InvertY() Error: Divide by 0" ); }
  if( v.Z ) v.Z = 1.f/v.Z; else { LOG( "FVector::InvertZ() Error: Divide by 0" ); }
  return v;
}

inline void Print( FString msg, FVector v )
{
  LOG( "%s %f %f %f", *msg, v.X, v.Y, v.Z );
}

inline void Print( FString msg, FBox box )
{
  LOG( "%s [%f %f %f] -> [%f %f %f], measuring %fx%fx%f, ext=(%fx%fx%f)", *msg, box.Min.X, box.Min.Y, box.Min.Z,
    box.Max.X, box.Max.Y, box.Max.Z,
    box.GetSize().X, box.GetSize().Y, box.GetSize().Z,
    box.GetExtent().X, box.GetExtent().Y, box.GetExtent().Z );
}

inline FVector& ZERO( FVector & v ) {
  v.X=v.Y=v.Z=0.f; return v;
}

inline int randInt( int max )
{
  if( max )
    return FMath::Rand()%max;
  else
    return max;
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

inline float randFloat( int max )
{
  return FMath::FRand()*max;
}

template <typename T> inline T& Clamp( T& a, const T& min, const T& max )
{
  if( a < min ) a = min;
  else if( a > max ) a = max;
  return a;
}

struct Ray
{
  FVector start, dir, end; // normalized direction vector
  float len;
  Ray()
  {
    start = FVector(0,0,0);
    dir = FVector(0,0,-1);
    len = 1.f;
    end = start + dir*len;
  }
  Ray(FVector origin, FVector direction) : start(origin), dir(direction)
  {
    len = dir.Size();
    if( !len ) {
      dir.Z = -1;
      len = 1.f;
    }
    dir /= len;
    end = start + dir*len;
  }
  
  Ray(FVector origin, FVector NDirection, float length) : start(origin), dir(NDirection)
  {
    // Check the normalized input is indeed normalized
    float NLen = dir.Size();
    if( !NLen ) {
      dir.Z = -1.f;
      NLen = 1.f;
    }
    dir /= NLen;
    
    len = length;
    end = start + dir*len;
  }
  void SetLen( float iLen ) {
    len = iLen;
    end = start + dir*len;
  }
  void Print( FString msg )
  {
    info( FS( "%s: Ray Start@(%f %f %f), Dir=(%f %f %f), len=%f, end=%f %f %f",
      *msg, start.X,start.Y,start.Z, dir.X,dir.Y,dir.Z, len, end.X,end.Y,end.Z ) );
  }
};

template <typename T> static T* GetComponentByType( AActor* a )
{
  TInlineComponentArray<T*> components;
	a->GetComponents(components);
  if( components.Num() ) return components[0];
  return 0;
}

template <typename T> static vector<T*> GetComponentsByType( AActor* a )
{
  TInlineComponentArray<T*> components;
	a->GetComponents(components);
  vector<T*> coll;
  for( int i = 0; i < components.Num(); i++ )
    coll.push_back( components[i] );
  return coll;
}

template <typename T> static T* GetComponentByName( AActor* a, FString name )
{
  TInlineComponentArray<T*> components;
	a->GetComponents(components);
  for( int i = 0; i < components.Num(); i++ )
    if( components[i]->GetName() == name )
      return components[i];
  return 0;
}

inline bool HasChildWithTag( AActor* actor, FName tag )
{
  if( actor && actor->ActorHasTag( tag ) )
    return 1;

  for( int i = 0; i < actor->Children.Num(); i++ )
  {
    AActor* child = actor->Children[i];
    if( child && HasChildWithTag( child, tag ) )
      return 1;
  }
  return 0;
}

inline void GetChildrenTagged( AActor* actor, set<AActor*>& tagged, FName fname )
{
  if( actor && actor->ActorHasTag( fname ) )
    tagged.insert( actor );
  for( int i = 0; i < actor->Children.Num(); i++ )
    GetChildrenTagged( actor->Children[i], tagged, fname );
}

inline void RemoveTagged( AActor* actor, FName fname )
{
  set<AActor*> tagged;
  GetChildrenTagged( actor, tagged, fname );
  for( AActor* a : tagged )
    a->Destroy();
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




