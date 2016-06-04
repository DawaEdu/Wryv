#pragma once

#include "UIActionCommand.h"
#include "UIInProgressUnitCounter.generated.h"

class ABuilding;
class AUnit;

class UUITrainingActionCommand;

// A UUIInProgressUnitCounter is a counter for a Unit that is being built.
// Distinct from TrainingAction, since the GO button cancels the Unit.
UCLASS( meta=(ShortTooltip="Something that is being built indicated") )
class WRYV_API UUIInProgressUnitCounter : public UUIActionCommand
{
  GENERATED_BODY()
public:
  // Unit type being trained.
  TSubclassOf<AUnit> UnitType;
  // Building that hosts the unit
  ABuilding* HostBuilding;
  
  UUIInProgressUnitCounter( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime();
  void Set( UUITrainingActionCommand* training );
  // Cancels the Unit being created (returns money).
  virtual bool Click();
  virtual void PopulateClock( Clock* inClock, int index );
  virtual void OnCooldown();
};
