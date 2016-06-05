#pragma once

#include "UI/HotSpot/Elements/Border.h"
#include "UI/HotSpot/Elements/BorderImage.h"
#include "UI/HotSpot/Elements/ProgressBar.h"

class AGameObject;

// Portrait of a unit (with border + mini hpbar)
class Portrait : public BorderImage
{
public:
  AGameObject* Object; // The object related to the portrait
  ProgressBar* hpBar;
  Solid* teamColor;

  Portrait( FString name, UTexture* tex, float thickness, FLinearColor color );
  void Set( AGameObject* go );
};


