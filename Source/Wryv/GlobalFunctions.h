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
#include <iterator>
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

// Removes all occurrences of `elt` from `v`. Returns # removals.
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

// Removes occurrences of `elt` from `v`. Returns #.
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

template <typename T> inline bool in( const vector<T*>& u, const vector<T*>& v )
{
  for( int i = 0; i < u.size(); i++ )
    for( int j = 0; j < v.size(); j++ )
      if( u[i] == v[j] )
        return 1;
  return 0;
}

template <typename T> inline bool in( const deque<T>& d, const T& elt )
{
  return find( d.begin(), d.end(), elt ) != d.end();
}

template <typename T, typename S> inline bool in( const map<T,S>& m, const T& key )
{
  return find( m.begin(), m.end(), elt ) != m.end();
}

// Linear search thru the map for an `S`
template <typename T, typename S> inline bool in( const map<T,S>& m, const S& val )
{
  for( const pair<const T,S>& p : m )
    if( p.second == val )
      return 1;
  return 0;
}

template <typename T, typename S> inline bool in( const map<T, vector<S> >& m, const S& val )
{
  for( const pair< const T, vector<S> > p : m )
    for( const S& s : p.second )
      if( s == val )
        return 1;
  return 0;
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

// Returns -1 if can't find an index which is a
template <typename T> int GetIndexWhichIsA( TArray<T*>& tarray, UClass* uclass )
{
  static_assert( is_base_of< UObject, T >::value, "GetIndexWhichIsA<T>: T must derive from UObject" );
  for( int i = 0; i < tarray.Num(); i++ )
    if( tarray[i]->IsA( uclass ) )
      return i;
  // not found
  return -1;
}

template <typename T> FString GetNames( const TArray<T*>& tarray )
{
  static_assert( is_base_of< UObject, T >::value, "GetNames<T>: T must derive from UObject" );
  FString fs;
  for( int i = 0; i < tarray.Num() - 1; i++ )
    fs += tarray[i]->GetName() + FString( ", " );
  if( tarray.Num() )
    fs += tarray[ tarray.Num() - 1 ]->GetName();

  return fs;
}

template <typename T> FString GetNames( const vector<T*>& v )
{
  static_assert( is_base_of< UObject, T >::value, "GetNames<T>: T must derive from UObject" );
  FString fs;
  for( int i = 0; i < (int)v.size() - 1; i++ )
    fs += v[i]->GetName() + FString( ", " );
  if( v.size() )
    fs += v[ v.size() - 1 ]->GetName();

  return fs;
}

template <typename T> FString GetNames( set<T*>& s )
{
  static_assert( is_base_of< UObject, T >::value, "GetNames<T>: T must derive from UObject" );
  FString fs;
  
  // With more than 2 elts, push in first N-1 elts
  if( s.size() >= 2 )
    for( set<T*>::iterator iter = s.begin(); iter != --s.end(); ++iter )
      fs += iter->GetName() + FString( ", " );
  
  // Push in last elt
  if( s.size() )
    fs += --s.end()->GetName(); // last one

  return fs;
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
    removeElement<T>( A, b ) ;
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

template <typename T> set<T*>& operator-=( set<T*>& A, const set<T*>& B )
{
  for( T* t : B )
    removeElement<T>( A, t );
  return A;
}

// Difference of two sets
template <typename T> set<T> operator-( const set<T>& A, const set<T>& B )
{
  set<T> diff;
  // Difference of two sets. If there's something in A that is not in B,
  // return what's in A that's not in B.
  //    A:  r,s,t      A:  r,s,t
  //    B:  d,e,f      B:  r,s,b
  // diff:  r,s,t   diff:  t

  for( const T& a : A )
    if( !in( B, a ) )
      diff.insert( a );
  return diff;
}

// Union
template <typename T> set<T> operator+( const set<T>& A, const set<T>& B )
{
  set<T> un = A;
  for( const T& b : B )
    un.insert( b );
  return un;
}

template <typename T> set<T> MakeSet( const vector<T>& A )
{
  set<T> B;
  for( const T& a : A )
    B.insert( a );
  return B;
}
template <typename T> TSet<T> MakeTSet( const TArray<T>& A )
{
  TSet<T> B;
  for( const T& a : A )
    B.Add( a );
  return B;
}
template <typename T> set<T> MakeSet( const TArray<T>& A )
{
  set<T> B;
  for( const T& a : A )
    B.insert( a );
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
  for( const T& a : A )
    B.push_back( a );
  return B;
}

template <typename T> TArray<T> MakeTArray( const set<T>& A )
{
  TArray<T> B;
  for( const T& a : A )
    B.Push( a );
  return B;
}

template <typename T, typename S> vector<T> MakeVectorT( const map<T,S>& m )
{
  vector<T> B;
  for( const pair<T,S>& p : m )
    B.push_back( p.first );
  return B;
}

template <typename T, typename S> vector<S> MakeVectorS( const map<T,S>& m )
{
  vector<S> B;
  for( const pair<T,S>& p : m )
    B.push_back( p.second );
  return B;
}

template <typename T, typename S> vector<S> MakeVectorS( const map<T, vector<S> >& m )
{
  vector<S> v;
  for( const pair< const float, vector<S> > p : objects )
    for( const S& s : p.second )
      v += s;
  return os;
}


inline FVector Rand()
{
  return FVector( FMath::FRand(), FMath::FRand(), FMath::FRand() );
}

inline FVector Rand( FVector min, FVector max )
{
  return min + FMath::FRand()*( max - min );
}

extern FVector Zero, UnitX, UnitY, UnitZ;

inline FString GetEnumName( TCHAR* enumType, int enumValue )
{
  const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, enumType, true);
  return EnumPtr->GetEnumText(enumValue).ToString();
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

inline UClass* GetUClass( const FStringClassReference& scr )
{
  UClass* uclass = scr.TryLoadClass<UClass>();
  if( !uclass )
  {
    warning( FS( "TryLoadClass for %s failed", *scr.ToString() ) );
    uclass = scr.ResolveClass();
    if( !uclass )
    {
      error( FS( "Couldn't load UClass for SCR %s via Resolve", *scr.ToString() ) );
    }
  }
  else
  {
    info( FS( "Loaded UClass %s from SCR %s", *uclass->GetName(), *scr.ToString() ) );
  }
  
  return uclass;
}

// 
template <typename T> T* Construct( const FStringClassReference& scr )
{
  UClass* uclass = GetUClass( scr );
  if( !uclass )
  {
    error( FS( "Couldn't construct object of type %s due to NULL UClass", *scr.ToString() ) );
    return 0;
  }

  T* object = NewObject<T>( GetTransientPackage(), uclass );
  return object;
}

template <typename T> T* Construct( UClass* uclass )
{
  T* object = NewObject<T>( GetTransientPackage(), uclass );
  return object;
}

inline void SetMeshColor( UMeshComponent* mesh, UObject* parent, FName parameter, FLinearColor color )
{
  for( int i = 0; i < mesh->GetNumMaterials(); i++ )
  {
    UMaterialInterface *mi = mesh->GetMaterial( i );
    if( UMaterialInstanceDynamic *mid = Cast< UMaterialInstanceDynamic >( mi ) )
    {
      //info( "The MID was created " );
      mid->SetVectorParameterValue( parameter, color );
    }
    else
    {
      //info( "The MID wasn't created " );
      mid = UMaterialInstanceDynamic::Create( mi, parent );
      FLinearColor defaultColor;
      if( mid->GetVectorParameterValue( parameter, defaultColor ) )
      {
        mid->SetVectorParameterValue( parameter, color );
        mesh->SetMaterial( i, mid );
        //info( "Setting mid param" );
      }
    }
  }
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

template <typename T> T* GetComponentByName( AActor* actor, FString name )
{
  TInlineComponentArray<T*> components;
	actor->GetComponents(components);
  for( int i = 0; i < components.Num(); i++ )
    if( components[i]->GetName() == name )
      return components[i];
  return 0;
}

inline bool Intersects( FBox& box, FVector& pt )
{
  return box.Min.X < pt.X   &&   pt.X < box.Max.X   &&
         box.Min.Y < pt.Y   &&   pt.Y < box.Max.Y   &&
         box.Min.Z < pt.Z   &&   pt.Z < box.Max.Z ;
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




