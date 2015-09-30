#pragma once

#include "Action.h"
#include "TrainingAction.generated.h"

class ABuilding;
class AGameObject;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A training action") )
class WRYV_API UTrainingAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf< AUnit > UnitType;
  
  virtual void Go(AGameObject* go);
  virtual void OnRefresh();
  virtual void OnComplete();
  
};
