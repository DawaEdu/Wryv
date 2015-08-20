#pragma once

#include "Wryv.h"
#include "Types.h"
#include "UnitsData.generated.h"

USTRUCT()
struct WRYV_API FUnitsDataRow : public FTableRowBase
{
  GENERATED_USTRUCT_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TEnumAsByte<Types> Type;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Name;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) UTexture* Portrait;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Description;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Quantity;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 StoneCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 ManaCost;
  // If this is a ground attack spell/property, then it doesn't require a gameobject target
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) bool GroundAttack;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AOERadius;
  // Repair costs a fraction of GoldCost, LumberCost, StoneCost per HP recovered.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairHPFractionCost;
  // How many seconds per HP recovered, when Repairing
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairRate;
  // Polymorphic property: Time it takes to build this thing, or regenerate capability (GenTime, Timeout)
  //   * Spell: It's regeneration time
  //   * Building: Time to build it.
  //   * Length of the effect (only used for time-limited things such as spells or items)
  //   * Time to mine (resource)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float TimeLength;
  // Attack and defense properties
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float SpeedMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float HpMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float Armor;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float SightRange;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AttackCooldown;
  // Range it can attack from (spells have attack range also)
  //   * For a Weapon instance, its range of the sword or spear
  //   * For a Spell, its the range of the spell itself.
  //   * For a Peasant, this is the range from which he mines
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AttackRange;
  // The range this unit can pick things up from.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float PickupRange;
  // TeamId id it belongs to
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 TeamId;
  // Food this structure supplies (Farms + townhall)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 FoodProvided;
  /// How much the unit or building uses
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 FoodUsed;
  // The shortcut key to activate this thing. FKey: autopopulates dialog with available keys.
  // The UnitsData of all capabilities of the SelectedUnit in the UI's shortcut keys are activated.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FKey ShortcutKey;
  // Required buildings to have built or possess to be able to build this object
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TArray< TEnumAsByte< Types > > Requirements;
  // The abilities this unit has, including ability to build, etc.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TArray< TEnumAsByte< Types > > Abilities;
  // The blueprint from which class instance came from
  //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) 
  UClass* uClass;

  FUnitsDataRow()
  {
    Type = NOTHING;
    Portrait = 0;
    Description = "Description";
    Quantity = 1;
    GoldCost = LumberCost = StoneCost = ManaCost = 0;
    GroundAttack = 0;
    AOERadius = 0.f;
    RepairHPFractionCost = 0.f;
    RepairRate = 0.f;
    TimeLength = 0.f;
    SpeedMax = 100.f;
    HpMax = 100.f;
    Armor = 1.f;
    SightRange = 10000.f;
    AttackDamage = 10.f;
    AttackCooldown = 2.f;
    AttackRange = 100.f;
    PickupRange = 100.f;
    TeamId = 0;
    FoodProvided = 0;
    FoodUsed = 1;
    TimeLength = 10.f;
    uClass = 0;
  }

  FUnitsDataRow operator+=( const FUnitsDataRow& row )
  {
    FUnitsDataRow r;
    r.GoldCost += row.GoldCost;
    r.LumberCost += row.LumberCost;
    r.StoneCost += row.StoneCost;
    r.TimeLength += row.TimeLength;
    r.SpeedMax += row.SpeedMax;
    r.HpMax += row.HpMax;
    r.Armor += row.Armor;
    r.SightRange += row.SightRange;
    r.AttackDamage += row.AttackDamage;
    r.AttackCooldown += row.AttackCooldown;
    r.AttackRange += row.AttackRange;
    return r;
  }

};

struct PowerUpTimeOut
{
  FUnitsDataRow traits;
  float timeRemaining;
  PowerUpTimeOut():timeRemaining(0.f){}
  PowerUpTimeOut( FUnitsDataRow &iTraits, float timeLength ) :
    traits( iTraits ), timeRemaining( timeLength ) { }
};

