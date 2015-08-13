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
  // Translates the box by V
  FBox2DU operator+( FVector2D v ){ return FBox2DU( Min+v, Max+v ); }
  FBox2DU operator-( FVector2D v ){ return FBox2DU( Min-v, Max-v ); }
  float left(){ return Min.X; }
	float right(){ return Max.X; }
	float top(){ return Min.Y; }
	float bottom(){ return Max.Y; }

  FVector2D TL() { return Min; }
  FVector2D TR() { return FVector2D( Max.X, Min.Y ); }
  FVector2D BR() { return Max; }
  FVector2D BL() { return FVector2D( Min.X, Max.Y ); }
  
  void print(FString msg) {
    LOG(  "%s fbox2du bounds min=(%f %f) max=(%f %f) meas=(%f %f)",
      *msg, Min.X, Min.Y, Max.X, Max.Y, Size().X, Size().Y );
  }
  FVector2D Size() { 
    return Max - Min;
  }
};

