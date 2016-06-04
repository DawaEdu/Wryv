#include "Wryv.h"

#include "CursorTexture.h"
#include "ImageHS.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

UTexture* ImageHS::NoTextureTexture = 0;

void ImageHS::ImageWidgetDefaults()
{
  Tex = 0;
  hotpoint = FVector2D(0,0);
  uv = FVector2D(1,1);
  Rotation = 0.f;
  PivotPoint = FVector2D(0,0);
}

ImageHS::ImageHS( FString name ) : HotSpot( name ) { 
  ImageWidgetDefaults();
}

ImageHS::ImageHS( FString name, UTexture* pic ) : HotSpot( name )
{
  ImageWidgetDefaults();
  Tex = pic;
  if( Tex ) {
    Size.X = Tex->GetSurfaceWidth();
    Size.Y = Tex->GetSurfaceHeight();
    Size -= Pad;
  }
}

ImageHS::ImageHS( FString name, UTexture* pic, FLinearColor color ) : HotSpot( name )
{
  ImageWidgetDefaults();
  Tex = pic;
  Color = color;
  if( Tex ){ // assign size from tex
    Size.X = Tex->GetSurfaceWidth();
    Size.Y = Tex->GetSurfaceHeight();
    Size -= Pad;
  }
}

// Size specified:
ImageHS::ImageHS( UTexture* pic, FVector2D size ) : HotSpot( "ImageHS", size )
{
  ImageWidgetDefaults();
  Tex = pic;
  if( Tex ) SetName( Tex->GetName() );
  // Don't assign size from tex
}

ImageHS::ImageHS( FString name, UTexture* pic, FVector2D size ) : HotSpot( name, size )
{
  ImageWidgetDefaults();
  Tex = pic;
}

ImageHS::ImageHS( FString name, UTexture* pic, FVector2D size, FLinearColor color ) : HotSpot( name, size )
{
  ImageWidgetDefaults(); 
  Tex = pic;
  Color = color;
  // Don't assign size from tex
}

void ImageHS::SetTexture( UTexture* texture )
{
  Tex = texture;
  dirty = 1;
}

void ImageHS::SetTexture( FCursorTexture cursorTexture )
{
  Tex = cursorTexture.Texture;
  hotpoint = cursorTexture.Hotpoint;
  dirty = 1;
}

void ImageHS::render( FVector2D offset )
{
  if( hidden ) return;
  if( !Tex )
  {
    // We have to remove this comment for normal ops because
    // sometimes we want to have null texes eg in SlotPanel items when
    // no item is present
    LOG( "Texture not set for ImageHS `%s`, setting to NULL texture", *Name );
    // render should not be called when the texture is hidden
    Tex = NoTextureTexture; //!! Possibly remove
  }

  // The renderPosition is just the computed position minus center hotpoint
  FVector2D renderPos = Pos() - hotpoint + offset;
  //info( FS( "%s: (%f, %f) (%f, %f)", *Name, renderPos.X, renderPos.Y, Size.X, Size.Y ) );
  // If hidden, do not draw
  ((AHUD*)Game->hud)->DrawTexture( Tex, renderPos.X, renderPos.Y, 
    Size.X, Size.Y, 0, 0, uv.X, uv.Y, Color,
    EBlendMode::BLEND_Translucent, 1.f, 0, Rotation, PivotPoint );
  
  HotSpot::render( offset );
}

