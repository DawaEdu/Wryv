#pragma once

#include "GameObjects/Things/Resources/Resource.h"
#include "ExplodableResource.generated.h"

class APeasant;

UCLASS()
class WRYV_API AExplodableResource : public AResource
{
  GENERATED_BODY()
public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )
  UDestructibleComponent* destructableMesh;
  // Time to wait before mesh cleanup
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)
  float MaxExplosionTime;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)
  float ExplosiveRadius;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)
  float ExplosiveForce;
  AExplodableResource(const FObjectInitializer& PCIP);
  virtual void BeginPlay() override;
  virtual void Die();
};
