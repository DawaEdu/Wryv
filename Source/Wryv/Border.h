#pragma once

#include "HotSpot.h"
#include "SolidWidget.h"

class Border : public HotSpot
{
  SolidWidget *left, *top, *bottom, *right;
public:
  FBox2DU Box;
  float Thickness;
  Border( FString name, FBox2DU box, float thickness, FLinearColor color ):
    HotSpot( name )
  {
    Thickness = thickness;

    left = new SolidWidget( "left", color );
    Add( left );
    top = new SolidWidget( "top", color );
    Add( top );
    bottom = new SolidWidget( "bot", color );
    Add( bottom );
    right = new SolidWidget( "right", color );
    Add( right );

    Set( box );
  }

  void Set( FBox2DU box )
  {
    Box = box;
    // TL                  TR
    //   +----------------+
    //   |+--------------+|
    //   ||              ||
    //   |+--------------+|
    //   +----------------+
    // BL                  BR
    // Set the widths
    // trbl
    top->SetByCorners( box.TL(), box.TR() + FVector2D(0,Thickness) );
    right->SetByCorners( box.TR() + FVector2D( -Thickness, 0 ), box.BR() );
    bottom->SetByCorners( box.BL() + FVector2D( 0, Thickness ), box.BR() );
    left->SetByCorners( box.TL(), box.BL() + FVector2D( Thickness, 0 ) );
  }
};

class MouseSelectBox : public Border
{
  FVector2D StartPt;
  ImageWidget* Cursor;

public:
  MouseSelectBox( FString name, FBox2DU box, float thickness, FLinearColor color ) :
    Border( name, box, thickness, color )
  {
  }

  void SetStart( FVector2D pt )
  {
    //LOG(  "box start (%f,%f)", pt.X, pt.Y );
    StartPt = pt;
    Box.Min = Box.Max = pt; // close box.
    Set( Box );
  }

  void SetEnd( FVector2D pt )
  {
    //LOG(  "box endpt (%f,%f)", pt.X, pt.Y );
    Box.Min = Box.Max = StartPt; // reset box
    Box += pt; // expand to contain new point.
    Set( Box );
  }
};


