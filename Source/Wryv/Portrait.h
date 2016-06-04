#pragma once

#include "Border.h"
#include "BorderImage.h"
#include "ProgressBar.h"

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


