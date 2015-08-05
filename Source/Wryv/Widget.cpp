#include "Wryv.h"

#include "Widget.h"
#include "GameFramework/HUD.h"
#include "TheHUD.h"
#include "GameObject.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

ATheHUD* HotSpot::hud = 0;

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
    LOG(  "Texture not set for ImageWidget `%s`", *Name );
    // render should not be called when the texture is hidden
  }

  // The renderPosition is just the computed position minus center hotpoint
  FVector2D renderPos = Pos() - hotpoint + offset;

  // If hidden, do not draw
  ((AHUD*)hud)->DrawTexture( Tex, renderPos.X, renderPos.Y, 
    Size.X, Size.Y, 0, 0, uv.X, uv.Y, Color,
    EBlendMode::BLEND_Translucent, 1.f, 0, Rotation, PivotPoint );
  FGenericPlatformMath::SRandInit(3); 
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
    LOG(  "The Canvas is not ready");
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
  FVector2D pos = Pos() + offset;
  hud->DrawText( Text, Color, pos.X, pos.Y, Font, Scale );
  HotSpot::render( offset );
}

Cooldown::Cooldown( UTexture *tex, FLinearColor color ) : ITextWidget( tex, "0%", BottomRight )
{
  building = CooldownCounter( Types::BLDGBARRACKS, 10.f );
  CreateClockMaterial();
}

Cooldown::Cooldown( UTexture *tex, FLinearColor color, CooldownCounter o ) : 
  ITextWidget( Game->unitsData[ o.Type ].Icon, "A", BottomRight ),
  building( o )
{
  // Spawn the clock material instance to apply to this widget
  CreateClockMaterial();
}

void Cooldown::CreateClockMaterial()
{
  clockMaterial = UMaterialInstanceDynamic::Create( Game->hud->ClockMaterialInstance, Game->hud );
  // Reset parameter values
  clockMaterial->SetScalarParameterValue( FName( "Percent" ), 0 );
  clockMaterial->SetVectorParameterValue( FName( "Color" ), Color );
}

void Cooldown::Tick( float t )
{
  // Update the Material's parameter
  building.Time += t;
  if( building.Done() ) building.Reset();
  clockMaterial->SetScalarParameterValue( FName( "Percent" ), building.Percent() );

  // Print the time remaining into the widget
  Set( FString::Printf( TEXT( "%.0f" ), building.Percent() ) );
  ITextWidget::Tick( t );

  // progress animations on clock faces
  ////overlay->SetAnimationPercent( building.Percent() );
}
  
void Cooldown::render( FVector2D offset )
{
  ITextWidget::render( offset );

  // Put the overlay on top.
  FVector2D pos = Pos() + offset;
  ((AHUD*)hud)->DrawMaterial( clockMaterial, pos.X, pos.Y, Size.X, Size.Y, 0, 0, 1, 1 );
}

void BuildQueue::Set( AGameObject* go )
{
  // Things that are spawning.
  for( int i = 0; i < go->buildQueue.size(); i++ )
  {
    Types type = go->buildQueue[i].Type;
    Cooldown *widget = new Cooldown( Game->unitsData[ type ].Icon,
      FLinearColor( 0.15, 0.15, 0.15, 0.15 ),
      CooldownCounter( type, Game->unitsData[ type ].BuildTime ) );
    StackRight( widget );

    // Register a function to remove the widget from this queue when clicked.
    widget->OnMouseDownLeft = [this,go,widget,i](FVector2D mouse)
    {
      // Remove this entry from the buildQueue then refresh the spawn queue
      // This works perfectly fine, the index to remove is actually
      // correct since a call to Set() follows removal from the queue.
      removeIndex( go->buildQueue, i );
      Set( go ); // reset the buildQueue's contents.
      return Consumed;
    };
  }
}

void GameChrome::Select( AGameObject *go )
{
  // on selection, populate the itemBelt and other widgets
  // Clear the items palette
  itemBelt->Clear();
  itemBelt->SetNumSlots( 0, 0 );

  // Set the build queue from this also
  buildQueue->Set( go );

  // Set abilities
  for( int i = 0; i < go->UnitsData.Abilities.Num(); i++ )
  {
    Types ability = go->UnitsData.Abilities[i];

    // Construct buttons that run abilities of the object.
    rightPanel->abilities->SetSlotTexture( i, Game->unitsData[ ability ].Icon );
  }
  
  for( int i = 0; i < go->UnitsData.Spawns.Num(); i++ )
  {
    Types spawn = go->UnitsData.Spawns[i];

    // Construct buttons that run abilities of the object.
    rightPanel->buildings->SetSlotTexture( i, Game->unitsData[ spawn ].Icon );
  }
}

