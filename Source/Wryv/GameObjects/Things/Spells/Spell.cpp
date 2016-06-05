#include "Wryv.h"

#include "UE4/Flycam.h"
#include "Util/GlobalFunctions.h"
#include "Spell.h"
#include "UE4/WryvGameInstance.h"
#include "UE4/WryvGameMode.h"

ASpell::ASpell(const FObjectInitializer& PCIP) : AProjectile(PCIP)
{
  
}

void ASpell::Damage( AGameObject* go, float t )
{
  go->Hp -= DamagePerSecond*t;
  if( Hp < 0 )  Hp = 0.f;
}

void ASpell::Move( float t )
{
  if( Arcs )
  {
    AProjectile::Move( t );
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


