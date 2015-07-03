#pragma once

#include "Building.h"
#include "Resource.generated.h"

UCLASS()
class RTSGAME_API AResource : public ABuilding
{
	GENERATED_UCLASS_BODY()
public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )
  int32 Amount;

  // Multiply the amount mined each turn by this amount.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )
  int32 Multiplier;

  //AResource(const FObjectInitializer& PCIP);
  void BeginPlay();
};
