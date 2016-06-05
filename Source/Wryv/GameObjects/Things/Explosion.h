#pragma once

#include "GameObjects/GameObject.h"
#include "Explosion.generated.h"

UCLASS()
class WRYV_API AExplosion : public AGameObject
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)
  UParticleSystemComponent* explosion;
  AExplosion( const FObjectInitializer& PCIP );
  virtual void PostInitializeComponents();
  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void OnEmitterFinished( UParticleSystemComponent* PSystem );
};
