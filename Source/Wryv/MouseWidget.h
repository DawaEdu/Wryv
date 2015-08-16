#pragma once

#include "Border.h"

class MouseSelectBox : public Border
{
  FVector2D StartPt;
  ImageWidget *Cursor;

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


class MouseWidget : public HotSpot
{
public:
  ImageWidget* cursor;
  MouseSelectBox* selectBox;

  MouseWidget( FString name, UTexture* tex ) : HotSpot( name )
  {
    selectBox = new MouseSelectBox( "MouseSelectBox border",
      FBox2DU( FVector2D(100,100), FVector2D(50,50) ),
      8.f, FLinearColor::Green);
    Add( selectBox );
    selectBox->Hide();

    cursor = new ImageWidget( name, tex );
    Add( cursor );
  }

  void Set( FVector2D mouse )
  {
    // Sets with mouse cursor position
    cursor->Margin = mouse;
  }

  void SelectStart( FVector2D mouse )
  {
    selectBox->SetStart( mouse );
    selectBox->Show();

    cursor->Hide();
  }

  void DragBox( FVector2D mouse )
  {
    selectBox->SetEnd( mouse );
  }

  void SelectEnd()
  {
    selectBox->Hide();
    cursor->Show();
  }
  
};

