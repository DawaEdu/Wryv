#include "Wryv.h"
#include "ImageWidget.h"
#include "TheHUD.h"

UTexture* ImageWidget::NoTextureTexture = 0;

void ImageWidget::ImageWidgetDefaults()
{
  Tex = 0;
  hotpoint = FVector2D(0,0);
  uv = FVector2D(1,1);
  Rotation = 0.f;
}

ImageWidget::ImageWidget( FString name ) : HotSpot( name ) { 
  ImageWidgetDefaults();
}

ImageWidget::ImageWidget( FString name, UTexture* pic ) : HotSpot( name )
{
  ImageWidgetDefaults();
  Tex = pic;
  if( Tex ) {
    Size.X = Tex->GetSurfaceWidth();
    Size.Y = Tex->GetSurfaceHeight();
  }
}

ImageWidget::ImageWidget( FString name, UTexture* pic, FLinearColor color ) : HotSpot( name )
{
  ImageWidgetDefaults();
  Tex = pic;
  Color = color;
  if( Tex ){ // assign size from tex
    Size.X = Tex->GetSurfaceWidth();
    Size.Y = Tex->GetSurfaceHeight();
  }
}

// Size specified:
ImageWidget::ImageWidget( UTexture* pic, FVector2D size ) : HotSpot( "ImageWidget", size )
{
  ImageWidgetDefaults();
  Tex = pic;
  if( Tex ) SetName( Tex->GetName() );
  // Don't assign size from tex
}

ImageWidget::ImageWidget( FString name, UTexture* pic, FVector2D size ) : HotSpot( name, size )
{
  ImageWidgetDefaults();
  Tex = pic;
}

ImageWidget::ImageWidget( FString name, UTexture* pic, FVector2D size, FLinearColor color ) : HotSpot( name, size )
{
  ImageWidgetDefaults(); 
  Tex = pic;
  Color = color;
  // Don't assign size from tex
}

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
    Tex = NoTextureTexture;
  }

  // The renderPosition is just the computed position minus center hotpoint
  FVector2D renderPos = Pos() - hotpoint + offset;

  // If hidden, do not draw
  ((AHUD*)hud)->DrawTexture( Tex, renderPos.X, renderPos.Y, 
    Size.X, Size.Y, 0, 0, uv.X, uv.Y, Color,
    EBlendMode::BLEND_Translucent, 1.f, 0, Rotation, PivotPoint );
  
  HotSpot::render( offset );
}


