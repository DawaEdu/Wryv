#pragma once

#include "GameObjects/Buildings/Building.h"
#include "Barracks.generated.h"

UCLASS()
class WRYV_API ABarracks : public ABuilding
{
  GENERATED_BODY()
public:
  ABarracks( const FObjectInitializer& PCIP );
};


