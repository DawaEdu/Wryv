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
  // The name of the unit.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Name;
  // The icon that appears when this object type is selectable as a widget
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) UTexture* Icon;
  // The tooltip or extended description of this thing
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Description;
  // How many of this thing is in this instance (used for items)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Quantity;
  // Build cost
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 StoneCost;
  // Casting cost
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 ManaCost;
  // Repair costs a fraction of GoldCost, LumberCost, StoneCost per HP recovered.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairHPFractionCost;
  // How many seconds per HP recovered, when repairing
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairRate;
  // Time it takes to build this thing, or regenerate capability (GenTime, Timeout)
  //   * Spell: It's regeneration time
  //   * Building: Time to build it.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 BuildTime;
  // Attack and defense properties
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 SpeedMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 HpMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Armor;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float SightRange;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 AttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 AttackCooldown;
  // Range it can attack from (spells have attack range also)
  //   * For a Weapon instance, its range of the sword or spear
  //   * For a Spell, its the range of the spell itself.
  //   * For a Peasant, this is the range from which he mines
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 AttackRange;
  // The range this unit can pick things up from.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 PickupRange;
  // Team id it belongs to
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Team;
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
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TArray< TEnumAsByte<Types> > Abilities;
  // The blueprint from which class instance came from
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) UClass* uClass;

  FUnitsDataRow()
  {
    Type = NOTHING;
    GoldCost = LumberCost = StoneCost = BuildTime = SpeedMax = HpMax = Armor = SightRange = 
      AttackDamage = AttackCooldown = AttackRange = Team = FoodProvided = FoodUsed = 0;
    Quantity = 1;
    TimeLength = 0.f;
    uClass = 0;
    PickupRange = 100;
    SpeedMax = 100;
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

  FString ToString()
  {
    FString fs;
    fs = FString::Printf(
      TEXT( "%s [%s]: %s goldcost=%d lumbercost=%d stonecost=%d buildtime=%d " ), 
      *Name, *Description, *GetEnumName( Type ),
      GoldCost, LumberCost, StoneCost, BuildTime );
    fs += FString::Printf( TEXT("speed=%d hp=%d armor=%d ")
      TEXT("sightrange=%d attackdamage=%d attackcooldown=%d ")
      TEXT("attackrange=%d"),
      SpeedMax, HpMax, Armor, SightRange, AttackDamage, AttackCooldown, AttackRange );
    return fs;
  }

};
