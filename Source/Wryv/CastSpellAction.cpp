#include "Wryv.h"

#include "CastSpellAction.h"
#include "CombatUnit.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Spell.h"
#include "TheHUD.h"
#include "UnitsData.h"
#include "WryvGameInstance.h"

UCastSpellAction::UCastSpellAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  AcquiresTarget = 0;
}

UTexture* UCastSpellAction::GetIcon()
{
  return Game->GetData( Spell ).Portrait;
}

float UCastSpellAction::GetCooldownTotalTime()
{
  return Game->GetData( Spell ).Cooldown;
}

bool UCastSpellAction::Click()
{
  if( AcquiresTarget )
  {
    // Queues spell for use
    Game->hud->NextSpell = this;

    // If the NextSpell is set, then the cursor is a crosshair
    Game->hud->SetCursorStyle( ATheHUD::CrossHairs, FLinearColor::Blue );
  }
  else
  {
    // Cast immediately @ AttackTarget.
    Caster->CastSpell( Spell );
  }
  return 1;
}

bool UCastSpellAction::Hover()
{
  UAction::Hover();
  return 1;
}
