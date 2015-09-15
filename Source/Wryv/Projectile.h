#pragma once

#include "GameObject.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class WRYV_API AProjectile : public AGameObject
{
  GENERATED_UCLASS_BODY()
public:
  AGameObject* Shooter;
  float Gravity;
  
  // Called when the game starts or when spawned
  virtual void ai( float t );
  virtual void Move( float t ) override;
  virtual void Hit( AGameObject* other );
  void SetDestinationArc( FVector start, FVector end, float speed, float h );
};
