#pragma once

#include "HotSpot.h"
#include "Solid.h"

class Border : public HotSpot
{
  Solid *left, *top, *bottom, *right;
public:
  FBox2DU Box;
  float Thickness;
  Border( FString name, FBox2DU box, float thickness, FLinearColor color );
  void Set( FBox2DU box );
};

class MouseSelectBox : public Border
{
  FVector2D StartPt;
  ImageHS* Cursor;
public:
  MouseSelectBox( FString name, FBox2DU box, float thickness, FLinearColor color );
  void SetStart( FVector2D pt );
  void SetEnd( FVector2D pt );
};


