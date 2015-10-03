#pragma once

#include "CooldownCounter.h"
#include "Action.generated.h"

// The Action series of objects creates
// buttons for ItemUse, other actions etc.
// Does NOT use a USTRUCT() FObject type
// so that I don't have to repeat the settings
class AGameObject;

UCLASS( Abstract, meta=(ShortTooltip="Abstract base for any Action type") )
class WRYV_API UAction : public UObject
{
  GENERATED_UCLASS_BODY()
public:
  // The icon for the action
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  FString Text;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UTexture* Icon;

  // This is the amount of time it takes to charge the Action after use.
  // Every UAction + Derivative has a cooldown counter, but not all of them use it (defaults 0).
  // The cooldown.TotalTime is taken from the UClass* data when the Object is instantiated.
  FCooldownCounter cooldown;
  
  // The shortcut key to activate this thing. FKey: autopopulates dialog with available keys.
  // The UnitsData of all capabilities of the SelectedUnit in the UI's shortcut keys are activated.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FKey ShortcutKey;

  // Applies the ability
  virtual void OnRefresh();
  virtual void OnComplete();
  // Returns true if the Action is ready to be used
  bool IsReady(){ return cooldown.Done(); }
  virtual void Step( float t );
};
