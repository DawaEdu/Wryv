#pragma once

#include "Wryv.h"
#include "Types.h"
#include "AttackProperties.generated.h"

USTRUCT()
struct WRYV_API FAttackProperties
{
  GENERATED_USTRUCT_BODY()
public:
  // Weapon properties
  // If attacks send a projectile, set object here.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TEnumAsByte< Types > ReleasedProjectileWeapon;
  // Which object type is spawned on contact (explosion). Must be specified for object types
  // that are projectiles
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TEnumAsByte< Types > OnContact;
  // If this is a ground attack spell/property, then it doesn't require a gameobject target
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) bool AttacksGround;
  // For projectiles with a height, how high does it curve
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float MaxTravelHeight;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AOERadius;
  // Attack damage is BaseAttackDamage + rand()%BonusAttackDamage
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 BaseAttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 BonusAttackDamage;
  // The weapon's chance to miss
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float MissPercent;
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

};

