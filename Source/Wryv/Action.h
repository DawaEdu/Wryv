#pragma once

#include "CooldownCounter.h"
#include "GlobalFunctions.h"
#include "Action.generated.h"

// UIAction.h is used by Runtime/Slate/Public/Framework/Commands/UIAction.h
// so we cannot use UIAction as our base class name.

// The Action series of objects creates
// buttons for ItemUse, other Actions etc.
// Does NOT use a USTRUCT() FObject type
// so you can Blueprint the class and re-use the
// same Blueprint in multiple Characters.
class Clock;
class AGameObject;
class IText;

UCLASS( Abstract, meta=(ShortTooltip="Abstract base for any Action type") )
class WRYV_API UAction : public UObject
{
  GENERATED_BODY()
public:
  // Text describing the Action
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  FString Text;
  // The amount of time it takes to charge the Action after use.
  // Every UAction + Derivative has a cooldown counter,
  // but not all of them use it (defaults 0).
  // The cooldown.TotalTime is taken from the UClass* data when the Object is instantiated.
  FCooldownCounter cooldown;
  // The shortcut key to activate this thing.
  // FKey: autopopulates dialog with available keys.
  // The UnitsData of all capabilities of the SelectedUnit in the UI's shortcut keys are activated.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  FKey ShortcutKey;
  // Stores the index of the UACTION used for item use invokation.
  // Most Actions invoke the i'th ENTRY in some array in the base
  // object that this Action belongs to. The Entry in the base array
  // is used for network comm invokation (player 2 invoked Item2 in
  // his list of items).
  int UActionIndex;
  // The cooldown clock associated with this icon.
  Clock* clock;

  UAction( const FObjectInitializer & PCIP );
  // Requires implementation in a subclass
  virtual UTexture* GetIcon();
  // Returns the amount of time this Action takes to cooldown.
  virtual float GetCooldownTotalTime() { return 0.f; }
  // Kicks off the activity of the UIActionCommand.
  virtual bool Click() { return 1; }
  virtual bool Hover() { return 1; }
  // Returns true if the Action is ready to be used
  bool IsReady(){ return cooldown.Done(); }
  virtual void Step( float t );
  // Index of the clock is passed here also.
  virtual void PopulateClock( Clock* inClock, int index );
  virtual void OnCooldown();
};


