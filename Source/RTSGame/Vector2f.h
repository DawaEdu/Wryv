#ifndef VECTOR2F_H
#define VECTOR2F_H

#include <stdio.h>
#include <stdlib.h>

float randFloat() {
  return (float)rand()/RAND_MAX;
}

float randFloat( float low, float high ) {
  return low + randFloat()*( high - low );
}

// 
// Simulate a "battle" between RTS units.
// 
struct Vector2f
{
  float x, y;
  Vector2f() : x(0.f),y(0.f) {}
  Vector2f( float a ) : x(a),y(a) {}
  Vector2f( float ix, float iy ):
    x(ix), y(iy) {}
  static Vector2f random(){ return Vector2f( randFloat(), randFloat() ); }
  static Vector2f random( const Vector2f& v1, const Vector2f& v2 ){
    return Vector2f( randFloat( v1.x, v2.x ), randFloat( v1.y, v2.y ) );
  }
  Vector2f operator+( const Vector2f& v ) const {
    return Vector2f( x + v.x, y + v.x );
  }
  Vector2f operator-( const Vector2f& v ) const {
    return Vector2f( x - v.x, y - v.y );
  }
  Vector2f operator*( float t ) const {
    return Vector2f( x*t, y*t );
  }
  Vector2f operator/( float t ) const {
    return Vector2f( x/t, y/t );
  }
  Vector2f& operator+=( const Vector2f& v ) {
    x += v.x, y += v.y;
    return *this;
  }
  Vector2f& operator-=( const Vector2f& v ) {
    x -= v.x, y -= v.y;
    return *this;
  }
  Vector2f& operator*=( const Vector2f& v ) {
    x *= v.x, y *= v.y;
    return *this;
  }
  Vector2f& operator/=( const Vector2f& v ) {
    x /= v.x, y /= v.y;
    return *this;
  }
  float dist(const Vector2f& v) const {
    return (*this-v).len();
  }
  float len() {
    return sqrtf( x*x + y*y );
  }
};

#endif