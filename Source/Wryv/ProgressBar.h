#pragma once

#include "SolidWidget.h"

class ProgressBar : public SolidWidget
{
public:
  SolidWidget* progress;
  ProgressBar( FString name, float height, FLinearColor backgroundColor, FLinearColor foregroundColor );
  void Set( float fraction );
  void SetColors( FLinearColor foreground, FLinearColor background );
  virtual void render( FVector2D offset ) override;

};