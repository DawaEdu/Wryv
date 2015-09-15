#pragma once

#include "Unit.h"
#include "CombatUnit.generated.h"

UCLASS()
class WRYV_API ACombatUnit : public AUnit
{
	GENERATED_UCLASS_BODY()

public:
  virtual void Move( float t ) override;
  virtual void ai( float t ) override;
};
