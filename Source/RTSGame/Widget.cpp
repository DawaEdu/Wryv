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
  SetName( "HotSpot" );
  //TooltipText = "Tip";

  Align = None; // Absolute positioning as default
  Layout = Pixels; // pixel positioning (can also use percentages of parent widths)
  hidden = 0;
  eternal = 1;
  displayTime = FLT_MAX; // Amount of time remaining before removal
  // assumes extremely large number (1e37 seconds which is practically infinite)
  Margin = Pad = FVector2D(0,0);
  Size = FVector2D(32,32);
  Dead = 0;
  Color = FLinearColor::White;
  Parent = 0;
  dirty = 1;
  //bubbleUp = 1; // events by default bubble up thru to the next widget

  OnHover = [this](FVector2D mouse){
    if( !TooltipText.IsEmpty() )
      TooltipWidget->Set( TooltipText );
    return 0;
  };
}

void ImageWidget::render( FVector2D offset )
{
  if( hidden ) return;
  if( !Tex )
  {
    // We have to remove this comment for normal ops because
    // sometimes we want to have null texes eg in slotpalette items when
    // no item is present
    UE_LOG( LogTemp, Warning, TEXT( "Texture not set for ImageWidget `%s`" ), *Name );
    // render should not be called when the texture is hidden
  }

  // The renderPosition is just the computed position minus center hotpoint
  FVector2D renderPos = Pos() - hotpoint;

  // If hidden, do not draw
  hud->DrawTexture( Tex, renderPos.X + offset.X, renderPos.Y + offset.Y, 
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
  if( dirty )
    Measure();  // when measure succeeds dirty=0
  FVector2D pos = Pos();
  hud->DrawText( Text, Color, pos.X + offset.X, pos.Y + offset.Y, Font, Scale );
  HotSpot::render( offset );
}

