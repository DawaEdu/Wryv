#include "Wryv.h"

#include "Action.h"
#include "Clock.h"
#include "SlotPalette.h"
#include "WryvGameInstance.h"

UTexture* SlotPalette::SlotPaletteTexture = 0;

SlotPalette::SlotPalette( FString name, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  ImageWidget( name, bkg ), Rows( 0 ), Cols( 0 ), EntrySize( entrySize )
{
  Pad = pad;
  // Init w/ # slots used in this palette
  // The stock size of the width is 100px/slot.
  // We re-calculate the slotsize though based on # slots used.
  Resize( rows, cols );
}

void SlotPalette::Blank()
{
  for( int i = 0; i < children.size(); i++ )
  {
    GetClock(i)->Reset();
  }
}

FVector2D SlotPalette::GetSlotPosition( int i )
{
  if( !Cols ) {
    error( "GetSlotPosition: No columns yet" );
    return FVector2D(0,0);
  }

  int row = i / Cols;
  int col = i % Cols;
  return FVector2D( col * EntrySize.X, row * EntrySize.Y );
}

Clock* SlotPalette::GetClock( int i )
{
  if( i < 0 || i >= children.size() )
  {
    LOG( "SlotPalette::GetClock(%d) oob", i );
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
  if( i < 0 || i >= children.size() ) {
    error( FS( "Index %d OOB", i ) );
    return;
  }
  
  // get the size of entry i
  Clock* clock = GetClock( i );
  FVector2D adjSize = GetAdjustedSize( clock->Size );
  FVector2D diff = EntrySize - adjSize; // Move the position from stock pos to adj pos
  clock->Margin = GetSlotPosition( i ) + diff/2;
}

void SlotPalette::Resize( int rows, int cols )
{
  Rows = rows;
  Cols = cols;

  // Add additional slots to make right size
  // The size of this widget set here.
  for( int i = children.size(); i < GetNumActiveSlots(); i++ )
  {
    // initialize a bunch of cooldown counters
    FString name = FString::Printf( TEXT("SlotPalette `%s`'s Clock %d"), *Name, i+1 );
    Clock *clock = new Clock( name, EntrySize, NoTextureTexture, Clock::DefaultColor, Alignment::CenterCenter );
    clock->Margin = FVector2D(0,0);
    Add( clock );
    AdjustPosition( i );
  }

  recomputeSizeToContainChildren();
  SetNumSlots( rows, cols );
}

void SlotPalette::SetNumSlots( int rows, int cols )
{
  // check to see if # slots available matches # requested
  // if there are not enough slots then add more.
  if( children.size() < rows*cols )
    Resize( rows, cols );
  HideChildren();

  Rows = rows;
  Cols = cols;
  for( int i = 0; i < GetNumActiveSlots(); i++ )
  {
    AdjustPosition( i );
    GetClock( i )->Show();
  }

  // Adjust UV coords of tile tex
  uv = FVector2D( Cols, Rows ) / FVector2D( 6.f, 6.f ); // The texture is 6x6 blocks

  // Change the size of the panel to being correct size to bound the children
  recomputeSizeToContainChildren();
}

void SlotPalette::render( FVector2D offset )
{
  if( hidden ) return;

  // re-align dirty child elements.
  //for( int i = 0; i < children.size(); i++ )
  //{
  //  Clock* clock = GetClock(i);
  //  if( clock->dirty )
  //  {
  //    AdjustPosition( i ); // adjust the position of the ith clock
  //  }
  //}

  ImageWidget::render( offset );
}

