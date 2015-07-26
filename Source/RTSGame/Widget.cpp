#include "RTSGame.h"

#include "Widget.h"
#include "GameFramework/HUD.h"

AHUD* HotSpot::hud = 0;

UTexture* ResourcesWidget::GoldTexture=0;
UTexture* ResourcesWidget::LumberTexture=0;
UTexture* ResourcesWidget::StoneTexture=0;

UTexture* SolidWidget::SolidWhiteTexture = 0;
UTexture* SlotPalette::SlotPaletteTexture = 0;

void SlotEntry::SetTexture( UTexture* tex, FVector2D originalPos )
{
  hidden = 0;
  Icon = tex;
  FVector2D texSize( tex->GetSurfaceWidth(), tex->GetSurfaceHeight() );

  // decrease the slot size by the padding
  FVector2D scales = Parent->EntrySize / (texSize + Parent->Pad);

  // icons are always square so no stretch occurs to src icon images
  float minScale = scales.GetMin();
  Size = texSize * minScale;  // multiply by smaller of two scales to shrink to fit.
  FVector2D diff = Parent->EntrySize - Size;
  Pos = originalPos + diff/2;
}

void SlotEntry::render( FVector2D offset )
{
  if( hidden ) return;

  //if( dirty )
  { // It seems in first calls to render(), Measure() does not properly measure text width
    TextQuantity->realignInParent();
    dirty = 0;
  }

  ImageWidget::render( offset );
}
