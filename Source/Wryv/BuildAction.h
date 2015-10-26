#pragma once

#include "Action.h"
#include "BuildAction.generated.h"

class ABuilding;
class APeasant;

// Peasants have build Actions.
UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Build action") )
class WRYV_API UBuildAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf< ABuilding > BuildingType;
  APeasant* Peasant;

  virtual UTexture* GetIcon() override;
  // Buildings have a cooldown time of 0s also
  virtual float GetCooldownTotalTime() override { return 0.f; }
  // The building type was selected for placement by the peasant indicated.
  virtual bool Click();
  virtual bool Hover();
};
