#pragma once

#include "Action.h"
#include "UnitsData.h"
#include "Research.generated.h"

class AGameObject;
struct Team;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A motion action") )
class WRYV_API UResearch : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  // The traits-bonuses that this research item supplies.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  FUnitsDataRow Traits;
  
  // The Unit types that are affected by this upgrade.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TArray< TSubclassOf<AGameObject> > AffectedUnitTypes;

  // This is a reference to another UResearch level that is the next level
  // after this one. If there is no next level, this becomes nul
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  UResearch* NextLevel;

  Team *team;

  virtual void Go( AGameObject* go ) override;
  // Runs after research is complete
  virtual void OnComplete() override;
};
