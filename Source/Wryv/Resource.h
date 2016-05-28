#pragma once

#include "GameObject.h"
#include "Resource.generated.h"

class APeasant;

UCLASS()
class WRYV_API AResource : public AGameObject
{
  GENERATED_BODY()
public:
  typedef TSubclassOf<AResource> /* as */ Type;

  float AmountRemaining;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Cosmetics )  USkeletalMeshComponent* Mesh;
  // Sets the tree to jiggling.
  UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Stats )  bool Jiggle;
  // This triggers the resource's destruction sequence to occur
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  bool ResourcesFinished;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 Quantity;
  
  AResource(const FObjectInitializer& PCIP);
  void BeginPlay() override;
  void PostInitializeComponents() override;
  void Harvest( APeasant* peasant );
  float ResourcesFraction() { return AmountRemaining / Quantity; }
};


