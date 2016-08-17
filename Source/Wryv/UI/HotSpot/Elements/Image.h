#pragma once

#include "UI/HotSpot/Elements/HotSpot.h"

struct FCursorTexture;

class Image : public HotSpot
{
public:
  static UTexture* NoTextureTexture;
  UTexture* Tex;
  FVector2D uv; // The maximum coordinates of the UV texturing
  FVector2D hotpoint; // Usually top left corner (0,0), meaning will render from topleft corner.
  // if its half size, then it will render from the center (such as when an imageWidget is being
  // click-dragged
  float Rotation;
  FVector2D PivotPoint; // the pivot about which the rotation is based
  void ImageWidgetDefaults();
  Image( FString name );
  Image( FString name, UTexture* pic );
  Image( FString name, UTexture* pic, FLinearColor color );

  // Size specified:
  Image( UTexture* pic, FVector2D size );
  Image( FString name, UTexture* pic, FVector2D size );
  Image( FString name, UTexture* pic, FVector2D size, FLinearColor color );
  virtual void SetTexture( UTexture* texture );
  virtual void SetTexture( FCursorTexture cursorTexture );

  virtual ~Image(){}
  virtual void render( FVector2D offset ) override ;
};

