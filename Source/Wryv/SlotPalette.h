#pragma once

#include "ImageWidget.h"

class Clock;

class SlotPalette : public ImageWidget
{
  HotSpot* Drag;
  int Rows, Cols;
public:
  FVector2D EntrySize;
  static UTexture* SlotPaletteTexture;

  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  SlotPalette( FString name, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  virtual ~SlotPalette(){}
  int GetNumSlots() { return Rows*Cols; }
  FVector2D GetSlotPosition( int i );
  Clock* GetSlot( int i );
  FVector2D GetAdjustedSize( FVector2D size );
  void AdjustPosition( int i );
  Clock* SetSlotTexture( int i, UTexture* tex );
  vector<Clock*> SetNumSlots( int rows, int cols );

};

