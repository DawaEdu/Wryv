#pragma once

// Inlined functions
#include "Engine.h"

#include <vector>
#include <set>
#include <map>
#include <stdarg.h>

using namespace std;

WRYV_API DECLARE_LOG_CATEGORY_EXTERN( K, Log, All );

#define FS(x,...) FString::Printf( TEXT( x ), __VA_ARGS__ )

// CALL: LOG( "Format string %d", intValue );
#define LOG(x, ...) UE_LOG( LogTemp, Warning, TEXT( x ), __VA_ARGS__ )

#define skip continue

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

template <typename T> void removeElement( vector<T*>& v, T* elt )
{
  for( int i = v.size()-1 ; i >= 0; i-- )
    if( v[i] == elt )
      v.erase( v.begin() + i );
}

template <typename T> bool removeElement( vector<T>& v, T& elt )
{
  for( int i = v.size()-1 ; i >= 0; i-- )
    if( v[i] == elt )
      v.erase( v.begin() + i );
}

template <typename T> void removeElement( set<T*>& v, T* elt )
{
  for( set<T*>::iterator it = v.begin(); it != v.end(); )
  {
    if( *it == elt )
      it = v.erase( it );
    else ++it;
  }
}

template <typename T> void removeElement( set<T>& s, T& elt )
{
  set<T>::iterator it = s.find( elt );
  if( it != s.end() )
    s.erase( it );
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

template <typename T> inline bool in( set<T*>& s, T* elt )
{
  return s.find( elt ) != s.end();
}

template <typename T> inline bool in( set<T>& s, T& elt )
{
  return s.find( elt ) != s.end();
}

template <typename T> inline int in( vector<T>& s, T& elt )
{
  for( int i = 0; i < s.size(); i++ )
    if( s[i] == elt )
      return i;
  return -1;
}
template <typename T> inline int in( vector<T*>& s, T* elt )
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

// AActor* derivative required
template <typename T> void DestroyAll( vector<T*> &v )
{
  for( int i = 0; i < v.size(); i++ )
    v[i]->Destroy();
  v.clear();
}

template <typename T> void DestroyAll( set<T*> &v )
{
  for( T* t : v )
    t->Destroy();
  v.clear();
}

// Adds B to A, removing duplicates
template <typename T> vector<T*>& operator+=( vector<T*>& A, const vector<T*>& B )
{
  for( T* b : B )
    if( in( A, b ) ) {
      LOG( "don't keep: b was in A" );
    }
    else A.push_back( b ) ;
  return A;
}

template <typename T> vector<T*>& operator-=( vector<T*>& A, const vector<T*>& B )
{
  for( T* b : B )
    removeElement( A, b ) ;
  return A;
}

template <typename T> set<T*>& operator+=( set<T*>& A, const set<T*>& B )
{
  for( T* b : B )
    A.insert( b );
  return A;
}

template <typename T> set<T*>& operator-=( set<T*>& A, const set<T*>& B )
{
  for( T* t : B )
    removeElement( A, t );
  return A;
}

template <typename T> vector<T*>& makeVector( const set<T*>& A )
{
  vector<T*> B;
  for( T* t : A )
    B.push_back( t );
  return B;
}

template <typename T> set<T*>& makeSet( const vector<T*>& A )
{
  set<T*> B;
  for( T* t : A )
    B.insert( t );
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
  LOG( "%s [%f %f %f] -> [%f %f %f]", *msg, box.Min.X, box.Min.Y, box.Min.Z,
    box.Max.X, box.Max.Y, box.Max.Z );
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

template <typename T> inline T& Clamp( T& a, const T& min, const T& max )
{
  if( a < min ) a = min;
  else if( a > max ) a = max;
  return a;
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




