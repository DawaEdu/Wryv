#pragma once

#include "GameObjects/Buildings/Building.h"
#include "Smith.generated.h"

UCLASS()
class WRYV_API ASmith : public ABuilding
{
  GENERATED_BODY()
public:
  // Turns on when the smith is researching
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cosmetics)
  UParticleSystemComponent* ChimneySmoke;
  ASmith( const FObjectInitializer& PCIP );
};
