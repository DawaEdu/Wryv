#pragma once

#include "Projectile.h"
#include "Spell.generated.h"

UCLASS()
class WRYV_API ASpell : public AProjectile
{
  GENERATED_UCLASS_BODY()
public:
  // Some properties of spells that projectiles don't have.
  // The spell arcs or doesn't. Spells that arc just follow a projectile arc.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  bool Arcs;
  // Spray type spells have a time-length they stay on for.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  float Duration;
  // Field-damage type spells deal damage over time.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  float DamagePerSecond;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  bool DamagesFriendly;
  // Cost of casting a particular spell
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  int32 ManaCost;

  // Drain HP from units that are being overlapped by this spell.
  float Lifetime;

  void Damage( AGameObject* go, float t );
  // Called when the game starts or when spawned
  virtual void Move( float t ) override;
  virtual void Hit( AGameObject* other );
};
