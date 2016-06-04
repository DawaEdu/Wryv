#pragma once

#include "UIActionCommand.h"
#include "UIInProgressResearchCounter.generated.h"

class UUIResearchCommand;

// A UUIInProgressUnitCounter is a counter for a Unit that is being built.
// Distinct from TrainingAction, since the GO button cancels the Unit.
UCLASS( meta=(ShortTooltip="A research") )
class WRYV_API UUIInProgressResearchCounter : public UUIActionCommand
{
  GENERATED_BODY()
public:
  UUIResearchCommand* research;

  UUIInProgressResearchCounter( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime();
  void Set( UUIResearchCommand* research );
  // Cancels the Unit being created (returns money).
  virtual bool Click();
  virtual void OnCooldown();
};
