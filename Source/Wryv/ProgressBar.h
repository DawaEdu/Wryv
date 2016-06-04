#pragma once

#include "Solid.h"

class ProgressBar : public Solid
{
public:
  Solid* progress;
  // The fill & unfill colors
  static FLinearColor Fill, Bkg;
  float FillFraction;

  ProgressBar( FString name, float height, int alignment );
  ProgressBar( FString name, float height, FLinearColor backgroundColor, FLinearColor foregroundColor );
  virtual void Reflow();
  void InitFill( FLinearColor color );
  void Set( float fraction );
  void SetColors( FLinearColor foreground, FLinearColor background );
  virtual void render( FVector2D offset ) override;

};