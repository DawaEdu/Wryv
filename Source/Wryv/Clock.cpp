#include "Wryv.h"

#include "Clock.h"
#include "TheHUD.h"
#include "FlyCam.h"
#include "WryvGameInstance.h"

FLinearColor Clock::DefaultColor( 1,1,1,1 );

Clock::Clock( FString name, FVector2D size, UTexture* tex, FLinearColor pieColor, Alignment alignment ) : 
  ClockColor( pieColor ),
  IText( name, tex, size, "0%", alignment )
{
  // Spawn the clock material instance to apply to this widget
  clockMaterial = CreateClockMaterial( ClockColor );
  PieFillFraction = 0.f;
  DisplayPercentageText = 1;
  // Keep reference collected copy of material.
  Game->hud->MaterialInstances.Add( clockMaterial );
}

Clock::~Clock()
{
  // Deallocate the clockmaterial from the global collection
  Game->hud->MaterialInstances.Remove( clockMaterial );
}

void Clock::Reset()
{
  IText::Reset();
  SetTexture( NoTextureTexture );
}

UMaterialInstanceDynamic* Clock::CreateClockMaterial( FLinearColor pieColor )
{
  // Creates from the instance
  UMaterialInstanceDynamic* clock = UMaterialInstanceDynamic::Create(
    Game->hud->ClockMaterialInstance, GetTransientPackage() );
  // Reset parameter values
  clock->SetScalarParameterValue( FName( "Fraction" ), .25f );
  clock->SetVectorParameterValue( FName( "Color" ), pieColor );
  return clock;
}

void Clock::SetFillFraction( float fraction )
{
  PieFillFraction = fraction;
  dirty = 1;
  DisplayPercentageText = 1; // Turns on text display %
}

void Clock::SetFillFraction( float fraction, Alignment textAlign )
{
  Text->Align = textAlign;
  SetFillFraction( fraction );
}

void Clock::SetText( FString text, Alignment textAlignment )
{
  Text->Set( text );
  Text->Align = textAlignment;
  DisplayPercentageText = 0; // Turns off text %
}

void Clock::render( FVector2D offset )
{
  if( hidden )
  {
    return ;
  }

  // Re-create the clock material if it went invalid
  if( !clockMaterial->IsValidLowLevel() )
  {
    warning( FS( "Clock material for %s became invalid, recreating", *Name ) );
    clockMaterial = CreateClockMaterial( ClockColor );
  }

  if( dirty )
  {
    if( clockMaterial->IsValidLowLevel() )
    {
      clockMaterial->SetScalarParameterValue( FName( "Fraction" ), PieFillFraction );
      if( DisplayPercentageText )
        IText::SetText( FS( "%.0f%%", PieFillFraction*100.f ) );
    }
    else
      error( "The MID clock material became invalid, couldn't set parameter" );
    dirty = 0;
  }

  // Render text beneath clock material
  IText::render( offset );

  // Put the overlay on top.
  FVector2D pos = Pos() + offset;

  if( clockMaterial->IsValidLowLevel() )
    ((AHUD*)Game->hud)->DrawMaterial( clockMaterial, pos.X, pos.Y, Size.X, Size.Y, 0, 0, 1, 1 );
  else
    error( "The MID clock material became invalid, cannot draw" );
}


