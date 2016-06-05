#pragma once

#include "GameObjects/Buildings/Building.h"
#include "Farm.generated.h"

UCLASS()
class WRYV_API AFarm : public ABuilding
{
  GENERATED_BODY()
public:
  AFarm( const FObjectInitializer& PCIP );
};
