#pragma once

#include "ITextWidget.h"

class UMaterialExpressionVectorParameter;

class Clock : public ITextWidget
{
public:
  UMaterialInstanceDynamic* clockMaterial;
  FLinearColor ClockColor;
  float PieFillFraction; // The fill level of the pie
  static FLinearColor DefaultColor;
  bool DisplayPercentageText;

  static UMaterialInstanceDynamic *CreateClockMaterial( FLinearColor pieColor );
  Clock( FString name, FVector2D size, UTexture* tex, FLinearColor pieColor, Alignment alignment );
  ~Clock();

  // Resets functionality & textures of the clock.
  virtual void Reset();
  // Clears the extra elements inside the widget that are non-stock to the widget (eg
  // all except the Text node in an ITextWidget)
  void SetFillFraction( float fraction );
  // Sets the % fill of the clock, as well as the text inside the clock
  void SetFillFraction( float fraction, Alignment textAlign );

  void SetText( FString text, Alignment textAlignment );
public:
  virtual void render( FVector2D offset ) override;
};
