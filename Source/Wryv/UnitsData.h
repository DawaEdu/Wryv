#pragma once

#include "Wryv.h"
#include "Types.h"
#include "UnitsData.generated.h"

USTRUCT()
struct WRYV_API FUnitsDataRow : public FTableRowBase
{
  GENERATED_USTRUCT_BODY()
public:
  // The UnitTYPE (strict set of types).
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TEnumAsByte<Types> Type;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Name;
  // The icon that appears when this object type is selectable as a widget 
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) UTexture* Portrait;
  // The tooltip or extended description of this thing
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Description;
  // How many of this thing is in this instance (used for items)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Quantity;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 StoneCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 ManaCost;
  // If a spell or attack is AOE, it can be cast on the floor
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) bool AOE;
  // Radius of the attack
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AOERadius;
  // Repair costs a fraction of GoldCost, LumberCost, StoneCost per HP recovered.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairHPFractionCost;
  // How many seconds per HP recovered, when Repairing
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairRate;
  // Time it takes to build this thing, or regenerate capability (GenTime, Timeout)
  //   * Spell: It's regeneration time
  //   * Building: Time to build it.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float BuildTime;
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
  // Polymorphic property:
  //   * Length of the effect (only used for time-limited things such as spells or items)
  //   * Time to mine (resource)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float TimeLength;
  // The shortcut key to activate this thing. FKey: autopopulates dialog with available keys.
  // The UnitsData of all capabilities of the SelectedUnit in the UI's shortcut keys are activated.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FKey ShortcutKey;
  // What this unit type spawns
  // Type of unit spawned when this widget is pressed.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TArray< TEnumAsByte<Types> > Spawns;
  // The abilities this unit has.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TArray< TEnumAsByte< Types > > Abilities;
  // Required buildings to have built or possess to be able to build this object
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TArray< TEnumAsByte< Types > > Requirements;
  // The blueprint from which class instance came from
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) UClass* uClass;

  FUnitsDataRow()
  {
    Type = NOTHING;
    Portrait = 0;
    Description = "Description";
    Quantity = 1;
    GoldCost = LumberCost = StoneCost = ManaCost = 0;
    AOE = 0;
    AOERadius = 0.f;
    RepairHPFractionCost = 0.f;
    RepairRate = 0.f;
    BuildTime = 0.f;
    SpeedMax = 10000.f;
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
    r.BuildTime += row.BuildTime;
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
  float time;
  FUnitsDataRow traits;
  PowerUpTimeOut():time(0.f){}
  PowerUpTimeOut( float t, FUnitsDataRow &iTraits ) :
    time( t ), traits( iTraits ) { }
  void Tick( float t ) { time += t; }
};

