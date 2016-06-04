#pragma once

#include "Action.h"
#include "InProgressBuilding.generated.h"

class ABuilding;
class APeasant;

// A UInProgressBuilding is a counter for a building that is being built.
UCLASS( meta=(ShortTooltip="Something that is being built indicated") )
class WRYV_API UInProgressBuilding : public UAction
{
  GENERATED_BODY()
public:
  // The building the in-progress counter is attached to.
  ABuilding* Building;
  APeasant* Peasant;

  UInProgressBuilding( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime();

  // Cancels the in-progress building.
  void SetBuilding(ABuilding* building);

  virtual bool Click();
  virtual void OnCooldown();
  virtual void Step( float t );

};
