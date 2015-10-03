#include "Wryv.h"

#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "Spell.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

ASpell::ASpell(const FObjectInitializer& PCIP) : AProjectile(PCIP)
{
  Duration = 1.f;
  Lifetime = 0.f;
}

void ASpell::Damage( AGameObject* go, float t )
{
  go->Hp -= DamagePerSecond*t;
  if( Hp < 0 )  Hp = 0.f;
}

void ASpell::Move( float t )
{
  Lifetime += t;
  if( Arcs )
  {
    AProjectile::Move( t ); // Calls flush, so we put it last
  }
  else
  {
    if( Lifetime >= Duration )
    {
      // Kill spells that don't arc after they exceed duration.
      Die();
    }
  }

  // Deal DPS to each overlap
  // Deal damage to the target, draining HP
  for( AGameObject* go : HitOverlaps )
  {
    // Anything in the hit overlaps bounds gets damaged
    if( DamagesFriendly && isAllyTo( go ) )
      Damage( go, t );
    else if( isEnemyTo( go ) )
      Damage( go, t );
  }

}

void ASpell::Hit( AGameObject* other )
{
  if( Arcs )
  {
    AProjectile::Hit( other );
  }
  // Otherwise spell terminates in time instead of arcing
}


