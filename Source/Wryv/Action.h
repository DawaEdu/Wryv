#pragma once

#include "GameFramework/Actor.h"
#include "CooldownCounter.h"
#include "Action.generated.h"

// TODO: Subclass each
UENUM()
enum Abilities
{
  Movement    UMETA(DisplayName = "Move"),
  Attack      UMETA(DisplayName = "Attack"),
  Stop        UMETA(DisplayName = "Stop"),
  HoldGround  UMETA(DisplayName = "Hold Ground"),

  CastSpell   UMETA(DisplayName = "Cast Spell"),
  UseItem     UMETA(DisplayName = "Use an Item"),
  DoResearch  UMETA(DisplayName = "Do some research"),
  Build       UMETA(DisplayName = "Build some object"), // object to build must be a TSubclassOf<ABuilding> ref
};

class AGameObject;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A motion action") )
class WRYV_API UAction : public UObject
{
  GENERATED_UCLASS_BODY()
public:
  // The icon for the action
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  FString Text;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  UTexture* Icon;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TEnumAsByte< Abilities > Ability;
  // This is the amount of time it takes to charge the Action after use.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  FCooldownCounter cooldown;
  // The object that owns this instance of UAction
  AGameObject* Object;
  
  // Applies the ability
  virtual void Go(AGameObject* go);
  virtual void OnRefresh();
  virtual void OnComplete();
  // Returns true if the Action is ready to be used
  bool IsReady(){ return cooldown.Done(); }
  virtual void Step( float t );
};
