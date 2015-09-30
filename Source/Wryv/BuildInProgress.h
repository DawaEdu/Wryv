#pragma once

#include "Action.h"
#include "BuildInProgress.generated.h"

class ABuilding;
class AGameObject;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A motion action") )
class WRYV_API UBuildInProgress : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  ABuilding* BuildingInProgress;
  
  virtual void Go(AGameObject* go);
  virtual void OnComplete();
};
