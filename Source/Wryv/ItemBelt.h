#pragma once

#include "SlotPanel.h"

class ABuilding;
class AUnit;

// set of slots for items.
class ItemBelt : public SlotPanel
{
public:
  ItemBelt( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( vector<AGameObject*> go );
  virtual void render( FVector2D offset ) override;
};
