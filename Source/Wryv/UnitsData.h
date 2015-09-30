#pragma once

#include "Wryv.h"
#include "UnitsData.generated.h"

class ABuilding;
class AExplosion;
class AGameObject;
class AItem;
class AProjectile;
class AUnit;

class UAction;
class UBuildAction;
class UItemAction;
class UResearch;
class UTrainingAction;

USTRUCT()
struct WRYV_API FUnitsDataRow : public FTableRowBase
{
  GENERATED_USTRUCT_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Name;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) UTexture* Portrait;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Description;
  
  // CreateBuilding-cost properties
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 StoneCost;

  // Cost of casting 
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 ManaCost;
  // The amount something has. For a resource, its the amount of resource in the mine etc.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Quantity;
  
  // Health
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float HpMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float Armor;
  
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
  // The rate at which a resource is gathered
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float GatheringRate;

  // Attack and defense properties
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float SpeedMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float SightRange;
  
  // Weapon properties: If attacks send a projectile, set object here.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TSubclassOf< AProjectile > ReleasedProjectileWeapon;
  // GameObject or derivative UCLASS
  
  // The object that gets spawned when the unit explodes (eg a building)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TSubclassOf< AExplosion > OnExploded;
  
  // If this is a ground attack spell/property, then it doesn't require a gameobject target
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) bool AttacksGround;
  // For projectiles with a height, how high does it curve
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float MaxTravelHeight;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AOERadius;
  // Attack damage is BaseAttackDamage + rand()%BonusAttackDamage
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 BaseAttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 BonusAttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AttackSpeedMultiplier;
  
  // The weapon's chance to miss
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float MissPercent;
  // Range it can attack from (spells have attack range also)
  //   * For a Weapon instance, its range of the sword or spear
  //   * For a Spell, its the range of the spell itself.
  //   * For a Peasant, this is the range from which he mines
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AttackRange;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float FollowFallbackDistance;
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

  // Construction requirements: things to have built or possess to be able to build this object
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< TSubclassOf< ABuilding > > Requirements;
  
  // The abilities this unit has, including ability to build, etc.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< TSubclassOf< UAction > > Abilities;
  // Buildings we can build using this unit
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< TSubclassOf< UBuildAction > > Builds;
  // List of types this building can train.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< TSubclassOf< UTrainingAction > > Trains;
  // Researches that this Building can do
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< TSubclassOf< UResearch > > Researches;
  // The items the unit starts carrying
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< TSubclassOf< UItemAction > > StartingItems;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) bool Invulnerability;
  
  FUnitsDataRow()
  {
    Name = "UNNAMED UNIT";
    Portrait = 0;
    Description = "NO DESCRIPTION";
    GoldCost = LumberCost = StoneCost = 0;
    ManaCost = 0;
    Quantity = 1;
    HpMax = 100.f;
    Armor = 0.f;
    RepairHPFractionCost = 1.f;
    RepairRate = 1.f;
    TimeLength = 10.f;
    GatheringRate = 1.f;
    SpeedMax = 100.f;
    SightRange = 250.f;
    
    AttacksGround = 0;
    MaxTravelHeight = 150.f;
    AOERadius = 100.f;
    BaseAttackDamage = 2;
    BonusAttackDamage = 5;
    AttackSpeedMultiplier = 1.f;
    MissPercent = 0.2f;
    AttackRange = 50.f;
    FollowFallbackDistance = 100.f;
    PickupRange = 50.f;
    TeamId = 1;
    FoodProvided = 0;
    FoodUsed = 1;
    Invulnerability = 0;
  }

  inline bool Add( bool a, bool b )
  {
    int ia( (int)a ), ib( (int)b );
    ia += ib;
    return static_cast<bool>( !! ia ); // !! required for bool conversion without the warning
  }
  FUnitsDataRow operator+=( FUnitsDataRow row )
  {
    FUnitsDataRow r = *this;
    r.Name += row.Name;
    //r.Portrait += row.Portrait;
    r.Description += row.Description;
    r.GoldCost += row.GoldCost;
    r.LumberCost += row.LumberCost;
    r.StoneCost += row.StoneCost;
    r.ManaCost += row.ManaCost;
    r.Quantity += row.Quantity;
    r.HpMax += row.HpMax;
    r.Armor += row.Armor;
    r.RepairHPFractionCost += row.RepairHPFractionCost;
    r.RepairRate += row.RepairRate;
    r.TimeLength += row.TimeLength;
    r.GatheringRate += row.GatheringRate;
    r.SpeedMax += row.SpeedMax;
    r.SightRange += row.SightRange;
    
    r.AttacksGround = Add( r.AttacksGround, row.AttacksGround );
    r.MaxTravelHeight += row.MaxTravelHeight;
    r.AOERadius += row.AOERadius;
    r.BaseAttackDamage += row.BaseAttackDamage;
    r.BonusAttackDamage += row.BonusAttackDamage;
    r.AttackSpeedMultiplier += row.AttackSpeedMultiplier;
    r.MissPercent += row.MissPercent;
    r.AttackRange += row.AttackRange;
    r.FollowFallbackDistance += row.FollowFallbackDistance;
    r.PickupRange += row.PickupRange;
    r.TeamId += row.TeamId;
    r.FoodProvided += row.FoodProvided;
    r.FoodUsed += row.FoodUsed;
    r.Invulnerability = Add( r.Invulnerability, row.Invulnerability );

    return r;
  }
};

struct PowerUpTimeOut
{
  AItem* Powerup;
  float timeRemaining;
  float timeInterval;  //application interval between Use() calls
  PowerUpTimeOut():Powerup(0),timeRemaining(0.f),timeInterval(0.f){}
  PowerUpTimeOut( AItem* item, float timeLength, float interval ) :
    Powerup( item ), timeRemaining( timeLength ), timeInterval( interval ) { }
};


