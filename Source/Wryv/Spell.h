#pragma once

#include "Projectile.h"
#include "Spell.generated.h"

UCLASS()
class WRYV_API ASpell : public AProjectile
{
  GENERATED_BODY()
public:
  // Some properties of spells that projectiles don't have.
  // The spell arcs or doesn't. Spells that arc just follow a projectile arc.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  bool Arcs;
  // Field-damage type spells deal damage over time.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  float DamagePerSecond;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  bool DamagesFriendly;
  // Cost of casting a particular spell
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  int32 ManaCost;
  
  ASpell(const FObjectInitializer& PCIP);
  void Damage( AGameObject* go, float t );
  // Called when the game starts or when spawned
  virtual void Move( float t ) override;
  virtual void Hit( AGameObject* other );
};
