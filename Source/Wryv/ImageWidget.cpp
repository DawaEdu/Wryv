#include "Wryv.h"
#include "ImageWidget.h"
#include "TheHUD.h"

void ImageWidget::render( FVector2D offset )
{
  if( hidden ) return;
  if( !Tex )
  {
    // We have to remove this comment for normal ops because
    // sometimes we want to have null texes eg in slotpalette items when
    // no item is present
    LOG( "Texture not set for ImageWidget `%s`, setting to NULL texture", *Name );
    // render should not be called when the texture is hidden
    Tex = NullTexture;
  }

  // The renderPosition is just the computed position minus center hotpoint
  FVector2D renderPos = Pos() - hotpoint + offset;

  // If hidden, do not draw
  ((AHUD*)hud)->DrawTexture( Tex, renderPos.X, renderPos.Y, 
    Size.X, Size.Y, 0, 0, uv.X, uv.Y, Color,
    EBlendMode::BLEND_Translucent, 1.f, 0, Rotation, PivotPoint );
  
  HotSpot::render( offset );
}


