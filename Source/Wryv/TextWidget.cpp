#include "Wryv.h"
#include "TextWidget.h"
#include "TheHUD.h"

void TextWidget::Measure()
{
  // ERROR IF THE HUD is not currently ready
  // to draw (ie canvas size won't be available)
  if( hud->Valid() ){
    dirty = 0;
  }
  else{
    LOG(  "The Canvas is not ready");
  }

  hud->GetTextSize( Text, Size.X, Size.Y, Font, Scale );
}

// We call Measure() each call to render() since text cannot be measured except when
// canvas is ready (ie during render() calls)
void TextWidget::render( FVector2D offset )
{
  if( hidden ) return;
  if( dirty )
    Measure();  // when measure succeeds dirty=0
  FVector2D pos = Pos() + offset;
  hud->DrawText( Text, Color, pos.X, pos.Y, Font, Scale );
  HotSpot::render( offset );
}

