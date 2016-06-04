#include "Wryv.h"
#include "Border.h"

Border::Border( FString name, FBox2DU box, float thickness, FLinearColor color ):
    HotSpot( name )
{
  Thickness = thickness;

  left = new Solid( "left", color );
  Add( left );
  top = new Solid( "top", color );
  Add( top );
  bottom = new Solid( "bot", color );
  Add( bottom );
  right = new Solid( "right", color );
  Add( right );

  Set( box );
}

void Border::Set( FBox2DU box )
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
  // The border sits inside the padding of the containing element
  top->SetByCorners( box.TL(), box.TR() + FVector2D(0,Thickness) );
  right->SetByCorners( box.TR() + FVector2D( -Thickness, 0 ), box.BR() );
  bottom->SetByCorners( box.BL() + FVector2D( 0, Thickness ), box.BR() );
  left->SetByCorners( box.TL(), box.BL() + FVector2D( Thickness, 0 ) );
}

MouseSelectBox::MouseSelectBox( FString name, FBox2DU box, float thickness, FLinearColor color ) :
    Border( name, box, thickness, color )
{
}

void MouseSelectBox::SetStart( FVector2D pt )
{
  //LOG(  "box start (%f,%f)", pt.X, pt.Y );
  StartPt = pt;
  Box.Min = Box.Max = pt; // close box.
  Set( Box );
}

void MouseSelectBox::SetEnd( FVector2D pt )
{
  //LOG(  "box endpt (%f,%f)", pt.X, pt.Y );
  Box.Min = Box.Max = StartPt; // reset box
  Box += pt; // expand to contain new point.
  Set( Box );
}
