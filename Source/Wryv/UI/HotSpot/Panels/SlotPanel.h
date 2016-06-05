#pragma once

#include "UI/HotSpot/Elements/ImageHS.h"
#include "Util/CooldownCounter.h"

class UUIActionCommand;
class Clock;

// SlotPanel is always populated with generic HotSpot* objects.

class SlotPanel : public ImageHS
{
  int Rows, Cols;
public:
  FVector2D EntrySize;
  static UTexture* SlotPanelTexture;
  // Drag & Drop for items from slots
  // The item we are dragging (subhotspot)
  SlotPanel( FString name, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  virtual ~SlotPanel(){}
  // Turn off all textures and functionality so clocks appear blank.
  void Blank();
  int GetNumActiveSlots() { return Rows*Cols; }
  FVector2D GetSlotPosition( int i );
  FVector2D GetAdjustedSize( FVector2D size );
  void AdjustPosition( int i );
  void Resize( int rows, int cols );
  void SetNumSlots( int rows, int cols );

  // Populates the SlotPanel with UUIActionCommand elts, starting from index `startIndex`
  // T will derive from UUIActionCommand
  template <typename T> void Populate( TArray< T* /*:UUIActionCommand*/ > elts, int startIndex )
  {
    Blank(); //Blank previous entries

    // Compile-time assert that ensures T derives from UUIActionCommand
    // We would use vector< UUIActionCommand >, only we can't Populate with a vector< UUIItemActionCommand >
    // even though UUIItemActionCommand : UUIActionCommand.
    static_assert( is_base_of< UUIActionCommand, T >::value, "Populate<T>: T must derive from UUIActionCommand" );
    int clockIndex = startIndex;
    for( int i = 0;   i < elts.Num()   &&   clockIndex < GetNumActiveSlots();   i++ )
    {
      clockIndex = startIndex + i; // offsetted element
      Clock* clock = (Clock*)GetChild(clockIndex);
      elts[i]->PopulateClock( clock, i );
    }

    recomputeSizeToContainChildren();
  }

  virtual void render( FVector2D offset );
};


