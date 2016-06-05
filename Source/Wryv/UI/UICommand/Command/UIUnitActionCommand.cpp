#include "Wryv.h"

#include "UI/UICommand/Command/UIUnitActionCommand.h"

#include "GameObjects/GameObject.h"
#include "Util/GlobalFunctions.h"
#include "UE4/TheHUD.h"
#include "GameObjects/Units/Unit.h"
#include "UE4/WryvGameInstance.h"

UUIUnitActionCommand::UUIUnitActionCommand( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Ability = NotSet;
}

bool UUIUnitActionCommand::Click()
{
  // Queue use of ability for all selected units.
  if( cooldown.Done() )
  {
    // Actions set the next ability in the HUD
    Game->hud->SetNextAbility( Ability.GetValue() );
    return 1;
  }
  else
  {
    info( FS( "Units cooldown %s wasn't ready to for usage", *GetName() ) );
    return 0;
  }
}
