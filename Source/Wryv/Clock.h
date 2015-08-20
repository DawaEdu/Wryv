#pragma once

#include "ITextWidget.h"

class UMaterialExpressionVectorParameter;

class Clock : public ITextWidget
{
public:
  FLinearColor clockColor;
  UMaterialInstanceDynamic* clockMaterial;

  static UMaterialInstanceDynamic *CreateClockMaterial(FLinearColor color);
  Clock( FString name, FVector2D size, UTexture* tex, FLinearColor pieColor );
  ~Clock();

  void Set( float fraction );
  virtual void render( FVector2D offset ) override;
};

