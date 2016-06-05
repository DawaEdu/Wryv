#pragma once

#include "GameObjects/GameObject.h"
#include "Projectile.generated.h"

class AExplosion;

UCLASS()
class WRYV_API AProjectile : public AGameObject
{
  GENERATED_BODY()
public:
  AGameObject* Shooter;
  float Gravity;

  // For projectiles with a height, how high does it curve
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float MaxTravelHeight;
  
  // If this is a ground attack spell/property, then it doesn't require a gameobject target
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) bool TargetsGround;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AOERadius;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) bool Homing;
  FVector Vel; // Cached velocity of the projectile

  // If the spell has an AOE set, then it is just a remote explosion 
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)
  TSubclassOf< AExplosion > OnImpact;
  
  AProjectile(const FObjectInitializer& PCIP);
  // Called when the game starts or when spawned
  virtual void ai( float t );
  virtual void Move( float t ) override;
  virtual void Hit( AGameObject* other );
  virtual void RecomputePath();
  void SetDestinationArc( FVector start, FVector end );
  virtual void Die() override;
};


