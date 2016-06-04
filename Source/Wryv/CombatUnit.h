#pragma once

#include "Unit.h"
#include "CombatUnit.generated.h"

class ASpell;

UCLASS()
class WRYV_API ACombatUnit : public AUnit
{
  GENERATED_BODY()
public:
  // Spells the unit can cast. Each shows up as a separate icon.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TArray< TSubclassOf< ASpell > > Spells;
  UPROPERTY() TArray< UUICastSpellActionCommand* > CountersSpells;

  ACombatUnit( const FObjectInitializer& PCIP );
  virtual void InitIcons();
  virtual void MoveCounters( float t ) override;
  virtual void OnUnselected() override;
  virtual void ai( float t ) override;
  virtual void Die();
  void CastSpell( TSubclassOf< ASpell > SpellClassType );
  void CastSpell( TSubclassOf< ASpell > SpellClassType, FVector groundLocation );
  
};
