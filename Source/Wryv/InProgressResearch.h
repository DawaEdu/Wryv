#pragma once

#include "Action.h"
#include "InProgressResearch.generated.h"

class UResearch;

// A UInProgressUnit is a counter for a Unit that is being built.
// Distinct from TrainingAction, since the GO button cancels the Unit.
UCLASS( meta=(ShortTooltip="A research") )
class WRYV_API UInProgressResearch : public UAction
{
  GENERATED_BODY()
public:
  UResearch* research;

  UInProgressResearch( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime();
  void Set( UResearch* research );
  // Cancels the Unit being created (returns money).
  virtual bool Click();
  virtual void OnCooldown();
};
