#pragma once

#include "GameObject.h"
#include "Building.generated.h"

UCLASS()
class RTSGAME_API ABuilding : public AGameObject
{
	GENERATED_UCLASS_BODY()
	
public:
  //ABuilding(const FObjectInitializer& PCIP);
  float buildProgress;    // When a unit is building, this is the % progress it is to completion.
  
  virtual void Tick( float t ) override;
};
