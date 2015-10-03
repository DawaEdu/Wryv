#include "Wryv.h"

#include "GameObject.h"
#include "GlobalFunctions.h"
#include "TheHUD.h"
#include "UnitAction.h"
#include "Unit.h"
#include "WryvGameInstance.h"

UUnitAction::UUnitAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Ability = NotSet;
}

void UUnitAction::Click()
{
  info( FS( "Using Action %s [%s] on %s", *Text,
    *GetEnumName( TEXT("Abilities"), Ability ), *Unit->Stats.Name ) );
  Game->hud->NextAbility = Ability;

  // depending on the action style, set color
  switch( Ability )
  {
    case Abilities::Attack:
      Game->hud->SetCursorStyle( ATheHUD::CrossHairs, FLinearColor::Red );
      break;
    case Abilities::Movement:
      Game->hud->SetCursorStyle( ATheHUD::CrossHairs, FLinearColor::Blue );
      break;
    case Abilities::Stop:
      break;
    case Abilities::HoldGround:
      break;
  }
}
