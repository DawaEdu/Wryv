#pragma once

#include "UI/HotSpot/Panels/StackPanel.h"

class BuffsPanel : public StackPanel
{
public:
  FVector2D BuffSize;

  BuffsPanel( FString name, UTexture* bkg );
  void Set( AGameObject* go );
};
