#pragma once

#include "GameObjects/Buildings/Building.h"
#include "Townhall.generated.h"

UCLASS()
class WRYV_API ATownhall : public ABuilding
{
  GENERATED_BODY()
public:
  ATownhall( const FObjectInitializer& PCIP );
};
