#pragma once

#include "Action.h"
#include "BuildAction.generated.h"

class ABuilding;
class APeasant;

// Peasants have build actions.
UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Build action") )
class WRYV_API UBuildAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf< ABuilding > BuildingType;
  
  // The building type was selected for placement by the peasant indicated.
  virtual void Click(APeasant* peasant);
};
