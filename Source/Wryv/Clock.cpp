#include "Wryv.h"
#include "Clock.h"
#include "WryvGameInstance.h"
#include "TheHUD.h"
#include "FlyCam.h"

Clock::Clock( FString name, FVector2D size, UTexture* tex, FLinearColor pieColor ) : 
  ITextWidget( name, tex, size, "0%", Alignment::BottomRight ), clockColor( pieColor )
{
  // Spawn the clock material instance to apply to this widget
  clockMaterial = CreateClockMaterial( pieColor );
  // Keep reference collected copy of material.
  Game->hud->MaterialInstances.Add( clockMaterial );
}

Clock::~Clock()
{
  // Deallocate the clockmaterial from the global collection
  Game->hud->MaterialInstances.Remove( clockMaterial );
}

UMaterialInstanceDynamic* Clock::CreateClockMaterial( FLinearColor pieColor )
{
  UMaterialInstanceDynamic* clock = UMaterialInstanceDynamic::Create( Game->hud->ClockMaterialInstance, Game->hud );
  // Reset parameter values
  clock->SetScalarParameterValue( FName( "Fraction" ), 0.f );
  clock->SetVectorParameterValue( FName( "Color" ), pieColor );
  return clock;
}

void Clock::Set( float fraction )
{
  // Progress animations on clock faces
  if( !clockMaterial->IsValidLowLevel() )
    clockMaterial = CreateClockMaterial( clockColor );
  else if( clockMaterial->IsValidLowLevel() )
    clockMaterial->SetScalarParameterValue( FName( "Fraction" ), fraction );
  FString fs = FS( "%.0f%%", fraction*100.f );
  // Print the time remaining into the widget
  ITextWidget::Set( fs );
}

void Clock::render( FVector2D offset )
{
  if( hidden ) return ;

  ITextWidget::render( offset );
  
  // Put the overlay on top.
  FVector2D pos = Pos() + offset;
  if( clockMaterial->IsValidLowLevel() )
  {
    ((AHUD*)hud)->DrawMaterial( clockMaterial, pos.X, pos.Y, Size.X, Size.Y, 0, 0, 1, 1 );
  }
  else
  {
    LOG( "Clock::render(): The MID became invalid" );
  }
}

