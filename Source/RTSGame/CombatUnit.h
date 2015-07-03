#pragma once

#include "Unit.h"
#include "CombatUnit.generated.h"

UCLASS()
class RTSGAME_API ACombatUnit : public AUnit
{
	GENERATED_UCLASS_BODY()
public:
  virtual void ai( float t );
};
