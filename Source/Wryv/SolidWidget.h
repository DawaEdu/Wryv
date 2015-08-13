#pragma once

#include "ImageWidget.h"

class SolidWidget : public ImageWidget
{
public:
  static UTexture* SolidWhiteTexture;
  SolidWidget( FString name, FLinearColor color ) :
    ImageWidget( name, SolidWhiteTexture, color )
  {
  }
  SolidWidget( FString name, FVector2D size, FLinearColor color ) : 
    ImageWidget( name, SolidWhiteTexture, size, color )
  {
  }
  virtual ~SolidWidget(){}
};

