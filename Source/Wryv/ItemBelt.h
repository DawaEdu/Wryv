#pragma once

#include "SlotPalette.h"

class ABuilding;
class AUnit;

// set of slots for items.
class ItemBelt : public SlotPalette
{
public:
  ItemBelt( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void ClearTextures();
  void Set( AGameObject* go );
  virtual void render( FVector2D offset ) override;
};
