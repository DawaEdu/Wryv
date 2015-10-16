#pragma once

#include "ImageWidget.h"
#include "CooldownCounter.h"

class UAction;
class Clock;

// SlotPalette is always populated with Clock* objects.

class SlotPalette : public ImageWidget
{
  int Rows, Cols;
public:
  FVector2D EntrySize;
  static UTexture* SlotPaletteTexture;
  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  SlotPalette( FString name, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  virtual ~SlotPalette(){}
  int GetNumActiveSlots() { return Rows*Cols; }
  FVector2D GetSlotPosition( int i );
  Clock* GetClock( int i );
  FVector2D GetAdjustedSize( FVector2D size );
  void AdjustPosition( int i );
  void Resize( int rows, int cols );
  void SetNumSlots( int rows, int cols );

  // Populates the SlotPalette with UAction elts, starting from index `startIndex`
  // T will derive from UAction
  template <typename T> void Populate( TArray< T* /*:UAction*/ > elts, int startIndex )
  {
    // Compile-time assert that ensures T derives from UAction
    // We would use vector< UAction >, only we can't Populate with a vector< UItemAction >
    // even though UItemAction : UAction.
    static_assert( is_base_of< UAction, T >::value, "Populate<T>: T must derive from UAction" );
    int clockIndex = startIndex;
    for( int i = 0;   i < elts.Num()   &&   clockIndex < GetNumActiveSlots();   i++ )
    {
      clockIndex = startIndex + i; // offsetted element
      elts[i]->PopulateClock( GetClock( clockIndex ), i );
    }

    recomputeSizeToContainChildren();
  }

  virtual void render( FVector2D offset );
};


