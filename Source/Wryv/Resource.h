#pragma once

#include "GameObject.h"
#include "Resource.generated.h"

class APeasant;

UCLASS()
class WRYV_API AResource : public AGameObject
{
	GENERATED_UCLASS_BODY()
public:
  int32 AmountRemaining;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 OriginalAmount;
  // Multiply the amount mined each turn by this amount.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 Multiplier;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  USkeletalMeshComponent* Mesh;
  // Sets the tree to jiggling.
  UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Stats )  bool Jiggle;
  // This triggers the resource's destruction sequence to occur
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  bool ResourcesFinished;

  //AResource(const FObjectInitializer& PCIP);
  void BeginPlay() override;
  void PostInitializeComponents() override;
  void Harvest( APeasant* peasant );
  float ResourcesFraction() { return AmountRemaining / OriginalAmount; }
};
