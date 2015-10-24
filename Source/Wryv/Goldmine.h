#pragma once

#include "ExplodableResource.h"
#include "Goldmine.generated.h"

UCLASS()
class WRYV_API AGoldmine : public AExplodableResource
{
	GENERATED_UCLASS_BODY()
public:
  //AGoldmine(const FObjectInitializer& PCIP);
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  USceneComponent* EntryPoint;

};


