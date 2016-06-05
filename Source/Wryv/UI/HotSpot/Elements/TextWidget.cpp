#include "Wryv.h"
#include "UI/HotSpot/Elements/TextWidget.h"
#include "UE4/TheHUD.h"

void TextWidget::Measure()
{
  // ERROR IF THE HUD is not currently ready
  // to draw (ie canvas size won't be available)
  if( Game->hud->isValid() )
  {
    Game->hud->GetTextSize( Text, Size.X, Size.Y, Font, Scale );
    //info( FS( "The text `%s` measures %f %f", *Text, Size.X, Size.Y ) );
    dirty = 0;
  }
  else
  {
    warning( FS( "TextWidget [%s]::The Canvas is not ready", *Name ) );
  }
}

// We call Measure() each call to render() since text cannot be measured except when
// canvas is ready (ie during render() calls)
void TextWidget::render( FVector2D offset )
{
  if( hidden ) return;
  if( dirty ) {
    Measure();
  }

  FVector2D pos = Pos() + offset;
  Game->hud->DrawText( Text, Color, pos.X, pos.Y, Font, Scale );
  HotSpot::render( offset );
}

