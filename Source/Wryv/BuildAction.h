#pragma once

#include "Action.h"
#include "BuildAction.generated.h"

class ABuilding;
class AGameObject;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A motion action") )
class WRYV_API UBuildAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf< ABuilding > BuildingType;
  
  virtual void Go(AGameObject* go);
  virtual void OnComplete();
};
