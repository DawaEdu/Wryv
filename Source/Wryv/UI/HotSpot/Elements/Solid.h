#pragma once

#include "UI/HotSpot/Elements/Image.h"

class Solid : public Image
{
public:
  static UTexture* SolidWhiteTexture;
  Solid( FString name, FLinearColor color ) :
    Image( name, SolidWhiteTexture, color )
  {
  }
  Solid( FString name, FVector2D size, FLinearColor color ) : 
    Image( name, SolidWhiteTexture, size, color )
  {
  }
  virtual ~Solid(){}
};

