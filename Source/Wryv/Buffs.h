#pragma once

#include "StackPanel.h"

class Buffs : public StackPanel
{
public:
  FVector2D BuffSize;

  Buffs( FString name, UTexture* bkg );
  void Set( AGameObject* go );
};
