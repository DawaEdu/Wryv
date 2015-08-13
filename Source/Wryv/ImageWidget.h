#pragma once

#include "HotSpot.h"

class ImageWidget : public HotSpot
{
public:
  static UTexture* NullTexture;
  UTexture* Tex;
  FVector2D uv; // The maximum coordinates of the UV texturing
  FVector2D hotpoint; // Usually top left corner (0,0), meaning will render from topleft corner.
  // if its half size, then it will render from the center (such as when an imageWidget is being
  // click-dragged
  float Rotation;
  FVector2D PivotPoint; // the pivot about which the rotation is based
  virtual void ImageWidgetDefaults(){
    Tex = 0;
    hotpoint = FVector2D(0,0);
    uv = FVector2D(1,1);
    Rotation = 0.f;
  }
  ImageWidget( FString name ) : HotSpot( name ) { 
    ImageWidgetDefaults();
  }
  ImageWidget( FString name, UTexture* pic ) : HotSpot( name )
  {
    ImageWidgetDefaults();
    Tex = pic;
    if( Tex ) {
      SetName( Tex->GetName() );
      Size.X = Tex->GetSurfaceWidth();
      Size.Y = Tex->GetSurfaceHeight();
    }
  }
  ImageWidget( FString name, UTexture* pic, FLinearColor color ) : HotSpot( name )
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
  ImageWidget( UTexture* pic, FVector2D size ) : HotSpot( "ImageWidget", size )
  {
    ImageWidgetDefaults();
    Tex = pic;
    if( Tex ) SetName( Tex->GetName() );
    // Don't assign size from tex
  }
  ImageWidget( FString name, UTexture* pic, FVector2D size ) : HotSpot( name, size )
  {
    ImageWidgetDefaults();
    Tex = pic;
  }
  ImageWidget( FString name, UTexture* pic, FVector2D size, FLinearColor color ) : HotSpot( name, size )
  {
    ImageWidgetDefaults(); 
    Tex = pic;
    Color = color;
    // Don't assign size from tex
  }
  
  virtual ~ImageWidget(){}
protected:
  virtual void render( FVector2D offset ) override ;
};

