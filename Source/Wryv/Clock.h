#pragma once

#include "ITextWidget.h"

class Clock : public ITextWidget
{
public:
  CooldownCounter counter; // The actual counter
  FLinearColor clockColor;
  UMaterialInstanceDynamic *clockMaterial; //anim param
  
  static UMaterialInstanceDynamic *CreateClockMaterial(FLinearColor color);
  Clock( FString name, FVector2D size, CooldownCounter o, FLinearColor pieColor );
  ~Clock();

  virtual void Move( float t ) override;
  virtual void render( FVector2D offset ) override;
};

