#pragma once

#include "ImageWidget.h"
#include "CooldownCounter.h"

class UAction;
class Clock;

class SlotPalette : public ImageWidget
{
  HotSpot* Drag;
  int Rows, Cols;
public:
  FVector2D EntrySize;
  static UTexture* SlotPaletteTexture;
  static FLinearColor DefaultColor;
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

  // T will derive from UAction
  template <typename T> vector<Clock*> Populate( vector< T* > elts )
  {
    // Compile-time assert that ensures T derives from UAction
    // We would use vector< UAction >, only we can't Populate with a vector< UItemAction >
    // even though UItemAction : UAction.
    static_assert( is_base_of< UAction, T >::value, "Populate<T>: T must derive from UAction" );

    vector<Clock*> clocks;
    for( int i = 0; i < elts.size()   &&   i < GetNumSlots(); i++ )
    {
      // Construct buttons that run abilities of the object.
      UTexture* tex = elts[i]->Icon;
      SetSlotTexture( i, tex );
      Clock* clock = GetSlot( i );
      clock->Set( elts[i]->cooldown.Fraction() );
      clock->Show();
      clocks.push_back( clock );
    }

    return clocks;
  }

};

