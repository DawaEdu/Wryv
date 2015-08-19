#pragma once

#include "ImageWidget.h"

class Minimap : public ImageWidget
{
  Border* borders;
public:
  static UTexture* MinimapTexture;
  Minimap( float borderSize, FLinearColor borderColor );
  virtual ~Minimap(){}
};
