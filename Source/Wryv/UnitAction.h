#pragma once

// Represents the action of a unit, such as Move, HoldGround, etc.
#include "Action.h"
#include "CooldownCounter.h"
#include "UnitAction.generated.h"

UENUM()
enum Abilities
{
  NotSet      UMETA(DisplayName = "NotSet"),
  Movement    UMETA(DisplayName = "Move"),
  Stop        UMETA(DisplayName = "Stop"),
  Attack      UMETA(DisplayName = "Attack"),
  HoldGround  UMETA(DisplayName = "Hold Ground"),
};

class AGameObject;
class AUnit;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Unit's basic action") )
class WRYV_API UUnitAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  // The icon for the action
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TEnumAsByte< Abilities > Ability;

  // The Unit to which this ability is attached.
  AUnit* Unit;

  // Applies the ability
  virtual void Click();
};
