#include "Wryv.h"

#include "UI/UICommand/Command/UICastSpellActionCommand.h"
#include "GameObjects/Units/CombatUnit.h"
#include "GameObjects/GameObject.h"
#include "Util/GlobalFunctions.h"
#include "GameObjects/Things/Spells/Spell.h"
#include "UE4/TheHUD.h"
#include "Game/UnitsData.h"
#include "UE4/WryvGameInstance.h"

UUICastSpellActionCommand::UUICastSpellActionCommand( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  AcquiresTarget = 0;
}

UTexture* UUICastSpellActionCommand::GetIcon()
{
  return Game->GetData( Spell ).Portrait;
}

float UUICastSpellActionCommand::GetCooldownTotalTime()
{
  return Game->GetData( Spell ).Cooldown;
}

bool UUICastSpellActionCommand::Click()
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

bool UUICastSpellActionCommand::Hover()
{
  UUIActionCommand::Hover();
  return 1;
}
