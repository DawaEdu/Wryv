#pragma once

#include "UI/UICommand/Command/UIActionCommand.h"
#include "UITrainingActionCommand.generated.h"

class ABuilding;
class AGameObject;
class AUnit;

// Clicking on one of these kicks off the training of a unit.
// The unit does NOT get created until the progress completes.
// 
UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A training action") )
class WRYV_API UUITrainingActionCommand : public UUIActionCommand
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf< AUnit > UnitType;
  
  // The building that is training the unit.
  ABuilding* Building;

  UUITrainingActionCommand( const FObjectInitializer & PCIP );
  // When you click this button, it kicks off building a unit of UnitType.
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime();
  virtual bool Click();
};
