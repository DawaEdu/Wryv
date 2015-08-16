#include "Wryv.h"
#include "SlotPalette.h"
#include "Clock.h"

FLinearColor SlotPalette::DefaultColor( 0.15, 0.15, 0.15, 0.75 );

SlotPalette::SlotPalette( FString name, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  ImageWidget( name, bkg ), Rows(rows), Cols(cols), EntrySize(entrySize)
{
  Pad = pad;
  // Init w/ # slots used in this palette
  // The stock size of the width is 100px/slot.
  // We re-calculate the slotsize though based on # slots used.
  Drag = 0;
  SetNumSlots( rows, cols );
}

FVector2D SlotPalette::GetSlotPosition( int i )
{
  int row = i / Cols;
  int col = i % Cols;
  return FVector2D( col * EntrySize.X, row * EntrySize.Y );
}

Clock* SlotPalette::GetSlot( int i )
{
  if( i < 0 || i >= children.size() )
  {
    LOG( "SlotPalette::GetSlot(%d) oob", i );
    return 0;
  }
  return (Clock*)children[i];
}

// Gets you the adjusted size dimensions of a size
// if slots are (100x100) and you send a (150x100) icon,
// the icon resizes to (100x66)
// Computed by:
//   scales = 100 / 150, 100 / 100 = (2/3, 1)
//   take the minimum scale and scale the icon down by it (mult)
//   (150x100) * (2/3,2/3) = (100x66)
// if the slots are (100x100) and you send a (75x60) texture
// then 
//   scales = 100 / 75, 100 / 60 = (4/3, 5/3)
// scale up:
//   (75,60) * (4/3,4/3) = (100x80) [min scale is correct value to use]
// (200x300) 
//   scales = 100 / 200, 100 / 300 = (1/2,1/3)
FVector2D SlotPalette::GetAdjustedSize( FVector2D size )
{
  // Calculate the % scale to scale up/down by
  FVector2D scales = EntrySize / size;
  // icons are always square so no stretch occurs to src icon images
  float minScale = scales.GetMin();
  // minScale works b/c if should scale up then its the min scale up to make it fit
  // if scale down, then its the min scale to make the larger side fit
  FVector2D adjSize = size * minScale;  // multiply by smaller of two scales to shrink to fit.
  return adjSize;
}

void SlotPalette::AdjustPosition( int i )
{
  if( i < 0 || i >= children.size() )  return;

  // get the size of entry i
  Clock* clock = GetSlot( i );
  FVector2D adjSize = GetAdjustedSize( clock->Size );
  FVector2D diff = EntrySize - adjSize; // Move the position from stock pos to adj pos
  clock->Margin = GetSlotPosition( i ) + diff/2;
}

Clock* SlotPalette::SetSlotTexture( int i, UTexture* tex )
{
  if( i < 0 || i >= children.size() ) {
    LOG( "SlotPalette::SetSlotTexture() invalid slot" );
    return 0;
  }
  if( !tex ) {
    LOG( "Texture not set in slotpalette %s", *Name );
    return 0;
  }
  FVector2D texSize( tex->GetSurfaceWidth(), tex->GetSurfaceHeight() );
  Clock* slot = GetSlot( i );
  slot->Tex = tex;
  // actual position of the icon in the slot is a bit different than just std layout pos
  AdjustPosition(i);
  // adjust it a little bit based on the slot entry
  return slot;
}

// Widget's by themselves are just hotspots
vector<Clock*> SlotPalette::SetNumSlots( int rows, int cols )
{
  Clear();    // Remove the old SlotEntries
  vector<Clock*> slots;
  Rows = rows;
  Cols = cols;
    
  // If there are NO slots, then return here
  if( !rows && !cols ) {
    Hide(); // hides the SlotPalette when empty, this is the usual side effect of emptying all slots
    LOG(  "SlotPalette %s has no entries", *Name );
    return slots; // don't change the size vars when 0 size because
    // it will corrupt the Size variables
  }

  int numSlots = rows*cols;
  // Measure the UV coordinates used since the texture is 6x6
  uv = FVector2D( cols/6., rows/6. ); // The texture is 6x6 blocks
    
  // The size of this widget set here.
  for( int i = 0; i < numSlots; i++ )
  {
    // initialize a bunch of cooldown counters
    FString name = FString::Printf( TEXT("SP `%s`'s Clock %d"), *Name, i+1 );
    Clock *clock = new Clock( name, EntrySize, CooldownCounter(), DefaultColor );
    Add( clock );
    AdjustPosition( i );
    slots.push_back( clock );
  }

  // Change the size of the panel to being correct size to bound the children
  recomputeSizeToContainChildren();
  return slots;
}