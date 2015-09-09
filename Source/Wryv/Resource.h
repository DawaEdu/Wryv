#pragma once

#include "GameObject.h"
#include "Resource.generated.h"

UCLASS()
class WRYV_API AResource : public AGameObject
{
	GENERATED_UCLASS_BODY()
public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 Amount;
  // Multiply the amount mined each turn by this amount.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 Multiplier;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  UStaticMeshComponent* StaticMesh;
  
  //AResource(const FObjectInitializer& PCIP);
  void BeginPlay();
};
