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

bool UUnitAction::Click()
{
  //Unit->UseAbility( UActionIndex );
  return 1;
}
