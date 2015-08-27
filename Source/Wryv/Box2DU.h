#pragma once

#include "Wryv.h"

struct FBox2DU : public FBox2D
{
  // +---------->
  // | min
  // |  TL----TR
  // |   |    |
  // |   |    |
  // v  BL----BR
  //          max
  FBox2DU( FVector2D min, FVector2D max )
  {
    Min = min;
    Max = max;
  }
  FBox2DU()
  {
    Min.X = Min.Y = 1e6f;
    Max.X = Max.Y = -1e6f;
  }
  FBox2DU( float s )
  {
    Min.X = Min.Y = -s;
    Max.X = Max.Y =  s;
  }
  // Translates the box by V
  FBox2DU operator+( FVector2D v ) const { return FBox2DU( Min+v, Max+v ); }
  FBox2DU operator-( FVector2D v ) const { return FBox2DU( Min-v, Max-v ); }
  float left() const { return Min.X; }
	float right() const { return Max.X; }
	float top() const { return Min.Y; }
	float bottom() const { return Max.Y; }

  FVector2D TL() const { return Min; }
  FVector2D TR() const { return FVector2D( Max.X, Min.Y ); }
  FVector2D BR() const { return Max; }
  FVector2D BL() const { return FVector2D( Min.X, Max.Y ); }
  
  void print(FString msg) const;
  FVector2D Size() const { return Max - Min; }
  bool Empty() const { return Min.Equals( Max, 1e-6f ); }
};

