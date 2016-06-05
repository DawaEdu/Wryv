#pragma once

#include "UI/HotSpot/Panels/SlotPanel.h"

class ABuilding;
class AUnit;

/// Set of slots for items.
class ItemBeltPanel : public SlotPanel
{
public:
  ItemBeltPanel( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( vector<AGameObject*> go );
  virtual void render( FVector2D offset ) override;
};
