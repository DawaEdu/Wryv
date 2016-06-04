#pragma once

#include "UIActionCommand.h"
#include "UIInProgressBuildingCounter.generated.h"

class ABuilding;

// A UUIInProgressBuildingCounter is a counter for a building that is being built.
UCLASS( meta=(ShortTooltip="Something that is being built indicated") )
class WRYV_API UUIInProgressBuildingCounter : public UUIActionCommand
{
  GENERATED_BODY()
public:
  // The building the in-progress counter is attached to.
  ABuilding* Building;
  APeasant* Peasant;

  UUIInProgressBuildingCounter( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime();

  // Cancels the in-progress building.
  void SetBuilding(ABuilding* building);

  virtual bool Click();
  virtual void OnCooldown();
  virtual void Step( float t );

};
