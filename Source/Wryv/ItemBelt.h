#pragma once

#include "SlotPalette.h"

// set of slots for items.
class ItemBelt : public SlotPalette
{
public:
  ItemBelt( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( AGameObject* go );
};
