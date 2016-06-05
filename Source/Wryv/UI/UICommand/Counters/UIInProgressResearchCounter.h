#pragma once

#include "UI/UICommand/Command/UIActionCommand.h"
#include "UIInProgressResearchCounter.generated.h"

class UUIResearchCommand;

// A UUIInProgressUnit is a counter for a Unit that is being built.
// Distinct from TrainingAction, since the GO button cancels the Unit.
UCLASS( meta=(ShortTooltip="A research") )
class WRYV_API UUIInProgressResearch : public UUIActionCommand
{
  GENERATED_BODY()
public:
  UUIResearchCommand* research;

  UUIInProgressResearch( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime();
  void Set( UUIResearchCommand* research );
  // Cancels the Unit being created (returns money).
  virtual bool Click();
  virtual void OnCooldown();
};
