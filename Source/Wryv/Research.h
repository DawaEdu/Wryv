#pragma once

#include "Action.h"
#include "UnitsData.h"
#include "Research.generated.h"

class ABuilding;
class AGameObject;
struct Team;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Research action") )
class WRYV_API UResearch : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UTexture* Icon;
  
  // The traits-bonuses that this research item supplies.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  FUnitsData Traits;
  
  // The Unit types that are affected by this upgrade.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TArray< TSubclassOf<AGameObject> > AffectedUnitTypes;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties) int32 StoneCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties) float ResearchTime;
  // This is a reference to another UResearch level that is the next level after this one.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  TSubclassOf<UResearch> NextLevel;
  // The building conducting the research
  ABuilding* Building;

  virtual UTexture* GetIcon() override { return Icon; }
  virtual float GetCooldownTotalTime() override { return ResearchTime; }
  virtual bool Click();

};
