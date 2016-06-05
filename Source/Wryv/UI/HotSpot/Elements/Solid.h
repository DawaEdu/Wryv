#pragma once

#include "UI/HotSpot/Elements/ImageHS.h"

class Solid : public ImageHS
{
public:
  static UTexture* SolidWhiteTexture;
  Solid( FString name, FLinearColor color ) :
    ImageHS( name, SolidWhiteTexture, color )
  {
  }
  Solid( FString name, FVector2D size, FLinearColor color ) : 
    ImageHS( name, SolidWhiteTexture, size, color )
  {
  }
  virtual ~Solid(){}
};

