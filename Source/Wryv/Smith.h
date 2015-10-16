#pragma once

#include "Building.h"
#include "Smith.generated.h"

UCLASS()
class WRYV_API ASmith : public ABuilding
{
	GENERATED_UCLASS_BODY()
  // Turns on when the smith is researching
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cosmetics)  UParticleSystemComponent* ChimneySmoke;
  
};
