#pragma once

#include "UIActionCommand.h"
#include "UIBuildActionCommand.generated.h"

class ABuilding;
class APeasant;

// Peasants have build Actions.
UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Build action") )
class WRYV_API UUIBuildActionCommand : public UUIActionCommand
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf< ABuilding > BuildingType;
  APeasant* Peasant;

  UUIBuildActionCommand( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  // Buildings have a cooldown time of 0s also
  virtual float GetCooldownTotalTime() override { return 0.f; }
  // The building type was selected for placement by the peasant indicated.
  virtual bool Click();
  virtual bool Hover();
};
