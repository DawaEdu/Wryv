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
  UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UnitProperties)  float Gravity;

  // Called when the game starts or when spawned
  virtual void BeginPlay() override;
  virtual void ai( float t );
  virtual void Move( float t ) override;
  void SetDestinationArc( FVector start, FVector end );
};
