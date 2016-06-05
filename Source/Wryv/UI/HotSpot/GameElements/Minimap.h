#pragma once

#include "UI/HotSpot/Elements/BorderImage.h"

class Minimap : public BorderImage
{
public:
  static UTexture* MinimapTexture;
  vector<FVector2D> pts;
  Minimap( float borderThickness, FLinearColor borderColor );
  virtual ~Minimap(){}
  virtual void render( FVector2D offset );
};


