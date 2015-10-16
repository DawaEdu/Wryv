#include "Wryv.h"

#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Item.h"
#include "ItemAction.h"
#include "TheHUD.h"
#include "Tooltip.h"
#include "Unit.h"
#include "UserInterface.h"
#include "WryvGameInstance.h"

UItemAction::UItemAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Quantity = 1;
}

UTexture* UItemAction::GetIcon()
{
  return Game->GetData( ItemClass ).Portrait;
}

float UItemAction::GetCooldownTotalTime()
{
  return Game->GetData( ItemClass ).Cooldown;
}

bool UItemAction::Click()
{
  // Trigger the gameobject to consume i'th item.
  // It has to be by index so that network send for consumption
  // is by index as well.
  if( cooldown.Done() )
  {
    info( FS( "%s ItemAction %s clicked", *AssociatedUnit->GetName(), *ItemClass->GetName() ) );
    AssociatedUnit->UseItem( UActionIndex );
    return 1;
  }
  else
  {
    info( FS( "ItemAction %s could not be used, %f seconds remain",
      *ItemClass->GetName(), cooldown.TotalTime - cooldown.Time ) );
    return 0;
  }
}

void UItemAction::Step( float t )
{
  UAction::Step( t );
}

void UItemAction::PopulateClock( Clock* inClock, int i )
{
  //info( FS( "Before populating clock
  UAction::PopulateClock( inClock, i );

  clock->OnHover = [this](FVector2D mouse) -> EventCode
  {
    // display a tooltip describing the current item.
    // or could add as a child of the img widget
    Game->hud->ui->gameChrome->tooltip->PositionAsChild( clock, HCenter|OnTopOfParent );
    // put the tooltip as a child of the slot
    return NotConsumed;
  };

  // add in the count
  clock->SetText( FS( "%d", Quantity ), Alignment::BottomRight );

}
