#pragma once

#include "UI/HotSpot/Elements/Image.h"

class FlowPanel : public Image
{
public:
  int Rows, Cols;

  FlowPanel( FString name, UTexture* bgTex, int rows, int cols, FVector2D size );
  virtual void Reflow() override;
  void Reflow( FVector2D size );

  // re-flow after each add
  virtual void PostAdd();
  virtual void render( FVector2D offset ) override;
};


