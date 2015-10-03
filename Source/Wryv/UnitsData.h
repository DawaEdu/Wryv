#pragma once

#include "Wryv.h"
#include "UnitsData.generated.h"

class ABuilding;
class AItem;

USTRUCT()
struct FUnitsData
{
  GENERATED_USTRUCT_BODY()

  // Properties.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Name;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) UTexture* Portrait;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Description;
  // TeamId id it belongs to
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 TeamId;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 StoneCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairRate;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float SightRange;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float SpeedMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float HpMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float Armor;
  // How many seconds per HP recovered, when Repairing
  // Attack damage is BaseAttackDamage + rand()%BonusAttackDamage
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 BaseAttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 BonusAttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AttackSpeedMultiply;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float MissPercent;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float FollowFallbackDistance;
  // Range it can attack from (spells have attack range also)
  //   * For a Spell, its the range of the spell itself.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float AttackRange;
  // The range this unit can pick things up from.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float PickupRange;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) bool Invulnerability;
  // How much the unit uses
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 FoodUsed;
  // Construction requirements: things to have built or possess to be able to build this object.
  // Usually these are lists of building classes you must have.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< TSubclassOf< ABuilding > > Requirements;

  FUnitsData()
  {
    // STATS.
    Name = "UNNAMED UNIT";
    Portrait = 0;
    Description = "DESCRIPTION";
    TeamId = 0;
    GoldCost = LumberCost = StoneCost = 0;
    RepairRate = 1.f;
    SightRange = 500.f;
    SpeedMax = 100.f;
    HpMax = 100.f;
    Armor = 1.f;
    BaseAttackDamage = 1;
    BonusAttackDamage = 1;
    AttackSpeedMultiply = 1.f;
    MissPercent = 0.2f;
    FollowFallbackDistance = 250.f;
    AttackRange = 500.f;
    PickupRange = 50.f;
    Invulnerability = 0;
    FoodUsed = 1;
  }

  FUnitsData operator+=( const FUnitsData& o )
  {
    FUnitsData copy = *this;
    copy.GoldCost   += o.GoldCost;
    copy.LumberCost += o.LumberCost;
    copy.StoneCost  += o.StoneCost;
    copy.RepairRate += o.RepairRate;
    copy.SightRange += o.SightRange;
    copy.SpeedMax   += o.SpeedMax;
    copy.HpMax      += o.HpMax;
    copy.Armor      += o.Armor;
    copy.BaseAttackDamage  += o.BaseAttackDamage;
    copy.BonusAttackDamage += o.BonusAttackDamage;
    copy.AttackSpeedMultiply += o.AttackSpeedMultiply;
    copy.MissPercent += o.MissPercent;
    copy.FollowFallbackDistance += o.FollowFallbackDistance;
    copy.AttackRange += o.AttackRange;
    copy.PickupRange += o.PickupRange;
    copy.Invulnerability |= o.Invulnerability;
    copy.FoodUsed    += o.FoodUsed;
    return copy;
  }
};


