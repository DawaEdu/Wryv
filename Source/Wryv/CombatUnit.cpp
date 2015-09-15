#include "Wryv.h"
#include "CombatUnit.h"

ACombatUnit::ACombatUnit( const FObjectInitializer& PCIP ):Super( PCIP )
{
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


