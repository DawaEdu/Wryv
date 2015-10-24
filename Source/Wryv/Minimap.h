#pragma once

#include "ImageBorder.h"

class Minimap : public ImageBorder
{
public:
  static UTexture* MinimapTexture;
  vector<FVector2D> pts;
  Minimap( float borderThickness, FLinearColor borderColor );
  virtual ~Minimap(){}
  virtual void render( FVector2D offset );
};


