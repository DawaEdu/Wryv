#pragma once

#include "Action.h"
#include "InProgressUnit.generated.h"

class ABuilding;
class AUnit;

// A UInProgressUnit is a counter for a Unit that is being built.
// Distinct from TrainingAction, since the GO button cancels the Unit.
UCLASS( meta=(ShortTooltip="Something that is being built indicated") )
class WRYV_API UInProgressUnit : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  // Unit type being trained.
  TSubclassOf<AUnit> UnitType;
  // The type of unit to spawn when the counter is done
  ABuilding* OwningBuilding;
  
  // Cancels the Unit being created (returns money).
  virtual void Cancel();
  virtual void OnComplete();
  virtual void Step( float t );
};
