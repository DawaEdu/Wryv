#pragma once

#include "GameObject.h"
#include "Building.generated.h"

UCLASS()
class WRYV_API ABuilding : public AGameObject
{
	GENERATED_UCLASS_BODY()
  //ABuilding(const FObjectInitializer& PCIP);
  float TimeBuilding;    // When a unit is building, this is the % progress it is to completion.
  virtual void Move( float t ) override;
  float percentBuilt(){ return TimeBuilding / Stats.TimeLength; }
  virtual void Die();
};
