#pragma once

#include "GameObject.h"
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
  //AExplosion(const FObjectInitializer& PCIP);
  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void OnEmitterFinished( UParticleSystemComponent* PSystem );
};
