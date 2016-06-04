#pragma once

#include "UIActionCommand.h"
#include "UICastSpellActionCommand.generated.h"

class ACombatUnit;
class ASpell;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="Action for casting a spell") )
class WRYV_API UUICastSpellActionCommand : public UUIActionCommand
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  TSubclassOf< ASpell > Spell;
  // The spell acquires a target from the UI to be cast
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  bool AcquiresTarget;
  ACombatUnit* Caster;

  UUICastSpellActionCommand( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime() override;
  virtual bool Click();
  virtual bool Hover();
};
