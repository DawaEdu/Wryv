#pragma once

#include "UIActionCommand.h"
#include "Cost.h"
#include "UnitsData.h"
#include "UIResearchCommand.generated.h"

class ABuilding;
class AGameObject;
struct Team;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A Research action") )
class WRYV_API UUIResearchCommand : public UUIActionCommand
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UTexture* Icon;
  
  // The traits-bonuses that this research item supplies.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  FUnitsData Traits;
  
  // The Unit types that are affected by this upgrade.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TArray< TSubclassOf<AGameObject> > AffectedUnitTypes;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties) FCost Cost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties) float ResearchTime;
  // This is a reference to another UUIResearchCommand level that is the next level after this one.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  TSubclassOf<UUIResearchCommand> NextLevel;
  // The building conducting the research
  ABuilding* Building;

  UUIResearchCommand( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override { return Icon; }
  virtual float GetCooldownTotalTime() override { return ResearchTime; }
  virtual bool Click();

};
