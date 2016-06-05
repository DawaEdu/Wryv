#pragma once

// Represents the action of a unit, such as Move, HoldGround, etc.
#include "UI/UICommand/Command/UIActionCommand.h"
#include "Util/CooldownCounter.h"
#include "Game/Enums.h"
#include "UIUnitActionCommand.generated.h"

class AGameObject;
class AUnit;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Unit's basic action") )
class WRYV_API UUIUnitActionCommand : public UUIActionCommand
{
  GENERATED_BODY()
public:
  // The icon for the action
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  TEnumAsByte< Abilities > Ability;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UTexture* Icon;

  UUIUnitActionCommand( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override { return Icon; }
  virtual bool Click();
};
