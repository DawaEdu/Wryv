#include "Wryv.h"

#include "UIItemActionCommand.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Item.h"
#include "TheHUD.h"
#include "Tooltip.h"
#include "Unit.h"
#include "UserInterface.h"
#include "WryvGameInstance.h"

UUIItemActionCommand::UUIItemActionCommand( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Quantity = 1;
}

UTexture* UUIItemActionCommand::GetIcon()
{
  return Game->GetData( ItemClass ).Portrait;
}

float UUIItemActionCommand::GetCooldownTotalTime()
{
  return Game->GetData( ItemClass ).Cooldown;
}

bool UUIItemActionCommand::Click()
{
  // Trigger the gameobject to consume i'th item.
  // It has to be by index so that network send for consumption
  // is by index as well.
  if( cooldown.Done() )
  {
    info( FS( "%s ItemAction %s clicked",
      *AssociatedUnit->GetName(), *ItemClass->GetName() ) );
    AssociatedUnit->UseItem( UUICmdActionIndex );
    return 1;
  }
  else
  {
    info( FS( "ItemAction %s could not be used, %f seconds remain",
      *ItemClass->GetName(), cooldown.TotalTime - cooldown.Time ) );
    return 0;
  }
}

void UUIItemActionCommand::Step( float t )
{
  UUIActionCommand::Step( t ); // we override the base functionality.
  // The cooldown is SET from the UNIT using the cooldown, and doesn't
  // step 
}

void UUIItemActionCommand::PopulateClock( Clock* inClock, int i )
{
  UUIActionCommand::PopulateClock( inClock, i );

  clock->OnHover = [this](FVector2D mouse) -> EventCode
  {
    // display a tooltip describing the current item.
    // or could add as a child of the img widget
    Game->hud->ui->gameChrome->tooltip->PositionAsChild( clock, HCenter|OnTopOfParent );
    // put the tooltip as a child of the slot
    return NotConsumed;
  };

  // add in the count, which will be the 2nd child of the clock
  // Update qty text
  if( clock->GetNumChildren() > 1 )
  {
    TextWidget* tw = (TextWidget*)clock->GetChild(1);
    tw->Set( Quantity );
  }
  else
  {
    error( FS( "The hotspot %s didn't have the qty elt", *clock->Name ) );
  }
  
}
