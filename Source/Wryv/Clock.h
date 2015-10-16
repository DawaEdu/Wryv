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

  // Clears the extra elements inside the widget that are non-stock to the widget (eg
  // all except the Text node in an ITextWidget)
  virtual void ClearExtras();
  void SetFillFraction( float fraction );
  // Sets the % fill of the clock, as well as the text inside the clock
  void SetFillFraction( float fraction, Alignment textAlign );

public:
  virtual void render( FVector2D offset ) override;
};

