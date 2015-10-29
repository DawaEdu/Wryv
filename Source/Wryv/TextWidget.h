#pragma once

#include "HotSpot.h"

class TextWidget : public HotSpot
{
public:
  UFont *Font;
  FString Text;
  float Scale;

  // During construction, we assign Text directly, then call Measure.
  // Construction is assumed to happen when the HUD is available.
  TextWidget( FString fs, int alignment=Alignment::TopLeft, UFont *font=0, float scale=1.f ) : 
    HotSpot( FString( "TextWidget " ) + fs ),
    Text( fs ), Font( font ), Scale( 1.f )
  {
    SetAndMeasure( fs );
  }
  virtual ~TextWidget(){}

  // Modifies text inside widget, but does not remeasure text size (until next draw call)
  void Set( FString text )
  {
    Text = text;
    dirty = 1;
    // We do not call Measure() here, instead we call it at the
    // next RENDER call, since if the HUD is not available, we cannot
    // measure the text.
    //Measure(); 
    // [[ cannot call measure here, since may happen
    //    during callback due to mouse motion etc. ]]
  }

  void Set( int v ) { Set( FString::Printf( TEXT( "%d" ), v ) ); }

  // Sets the text and re-measures the size of the text immediately afterwards
  // the HUD must be ready for this
  void SetAndMeasure( FString text )
  {
    Set( text );
    Measure();
  }

  void Measure();
  
protected:
  // We call Measure() each call to render() since text cannot be measured except when
  // canvas is ready (ie during render() calls)
  virtual void render( FVector2D offset ) override;
};

