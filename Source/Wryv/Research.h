#pragma once

#include "Action.h"
#include "Research.generated.h"

class ABuilding;
class AGameObject;
struct Team;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Research action") )
class WRYV_API UResearch : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  // The traits-bonuses that this research item supplies.
  //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  //FUnitsDataRow Traits;
  
  // The Unit types that are affected by this upgrade.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TArray< TSubclassOf<AGameObject> > AffectedUnitTypes;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 StoneCost;

  // This is a reference to another UResearch level that is the next level
  // after this one. If there is no next level, this becomes nul
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf<UResearch> NextLevel;

  // The building conducting the research
  ABuilding* Building;

  virtual void Click();

  // Runs after research is complete
  virtual void OnComplete() override;
};
