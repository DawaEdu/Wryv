#include "Wryv.h"

#include "CombatUnit.h"
#include "Spell.h"
#include "WryvGameInstance.h"

#include "CastSpellAction.h"

ACombatUnit::ACombatUnit( const FObjectInitializer& PCIP ):Super( PCIP )
{
}

void ACombatUnit::InitIcons()
{
  AUnit::InitIcons();
  for( int i = 0; i < Spells.Num(); i++ )
  {
    UCastSpellAction* action = NewObject< UCastSpellAction >( this, Spells[i] );
    action->Caster = this;
    CountersSpells.push_back(  );
  }
}

void ACombatUnit::Move( float t )
{
  AUnit::Move( t );
}

void ACombatUnit::ai( float t )
{
  // Seek out opponents in groups.
  // A footman from the enemy group goes out randomly
  // and seeks the opponent.
  // Search for enemy units that are a distance away from the
  // player team. These units are out of vision range of this
  // unit, so we'll start making our way towards an enemy unit
  // Archers also seek out targets, but they prefer
  // units that are not ranged as well. If there are no
  // unranged units, seek a ranged unit instead.

  // Try to find an enemy unit to attack.
  AttackTarget = GetClosestEnemyUnit();
  
}

void ACombatUnit::CastSpell( TSubclassOf< ASpell > SpellClassType )
{
  ASpell* spell = Game->Make< ASpell >( SpellClassType, team, Pos );
  spell->Attack( AttackTarget );
}

void ACombatUnit::CastSpell( TSubclassOf< ASpell > SpellClassType, FVector groundLocation )
{
  
}


