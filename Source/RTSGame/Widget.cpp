#include "RTSGame.h"

#include "Widget.h"
#include "GameFramework/HUD.h"
#include "MyHUD.h"

AMyHUD* HotSpot::hud = 0;

UTexture* ResourcesWidget::GoldTexture=0;
UTexture* ResourcesWidget::LumberTexture=0;
UTexture* ResourcesWidget::StoneTexture=0;

UTexture* SolidWidget::SolidWhiteTexture = 0;
UTexture* SlotPalette::SlotPaletteTexture = 0;
UTexture* StackPanel::StackPanelTexture = 0;

TextWidget* HotSpot::TooltipWidget = 0;

void HotSpot::defaults()
{
  //TooltipText = "Tip";
  Align = None; // Absolute positioning as default
  Layout = Pixels; // pixel positioning (can also use percentages of parent widths)
  hidden = 0;
  displayTime = 0.f;
  Margin = Pad = Pos = FVector2D(0,0);
  Size = FVector2D(32,32);
  Color = FLinearColor::White;
  Parent = 0;
  dirty = 1;
  bubbleUp = 1; // events by default bubble up thru to the next widget

  OnHover = [this](FVector2D mouse){
    if( !TooltipText.IsEmpty() )
      TooltipWidget->Set( TooltipText );
    return 0;
  };
}
void ImageWidget::render( FVector2D offset )
{
  if( hidden ) return;
  FVector2D renderPos = Pos - hotpoint;
  if( !Icon )
  {
    // We have to remove this comment for normal ops because
    // sometimes we want to have null texes eg in slotpalette items when
    // no item is present
    UE_LOG( LogTemp, Warning, TEXT( "Texture not set for ImageWidget `%s`" ), *FString(Name.c_str()) );
    // render should not be called when the texture is hidden
  }

  // If hidden, do not draw
  hud->DrawTexture( Icon, renderPos.X + offset.X, renderPos.Y + offset.Y, 
    Size.X, Size.Y, 0, 0, uv.X, uv.Y, Color,
    EBlendMode::BLEND_Translucent, 1.f, 0, Rotation, PivotPoint );
    
  HotSpot::render( offset );
}

void TextWidget::Measure()
{
  // ERROR IF THE HUD is not currently ready
  // to draw (ie canvas size won't be available)
  if( hud->Valid() ){
    dirty = 0;
  }
  else{
    UE_LOG( LogTemp, Warning, TEXT( "The Canvas is not ready" ) );
  }

  hud->GetTextSize( Text, Size.X, Size.Y, Font, Scale );
}
  
// We call Measure() each call to render() since text cannot be measured except when
// canvas is ready (ie during render() calls)
void TextWidget::render( FVector2D offset )
{
  if( hidden ) return;
  if( dirty ) { // It seems in first calls to render(), Measure() does not properly measure text width
    Measure();
  }
  hud->DrawText( Text, Color, Pos.X + offset.X, Pos.Y + offset.Y, Font, Scale );
  HotSpot::render( offset );
}


void SlotEntry::SetTexture( UTexture* tex, FVector2D originalPos )
{
  hidden = 0;
  Icon = tex;
  FVector2D texSize( tex->GetSurfaceWidth(), tex->GetSurfaceHeight() );

  // decrease the slot size by the padding
  FVector2D scales = Palette->EntrySize / (texSize + Parent->Pad);

  // icons are always square so no stretch occurs to src icon images
  float minScale = scales.GetMin();
  Size = texSize * minScale;  // multiply by smaller of two scales to shrink to fit.
  FVector2D diff = Palette->EntrySize - Size;
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
