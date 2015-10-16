#pragma once

// Represents the action of a unit, such as Move, HoldGround, etc.
#include "Action.h"
#include "CooldownCounter.h"
#include "Enums.h"
#include "UnitAction.generated.h"

class AGameObject;
class AUnit;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Unit's basic action") )
class WRYV_API UUnitAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  // The icon for the action
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  TEnumAsByte< Abilities > Ability;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UTexture* Icon;
  // The Unit to which this ability is attached.
  AUnit* Unit;

  virtual UTexture* GetIcon() override { return Icon; }
  virtual bool Click();
};
