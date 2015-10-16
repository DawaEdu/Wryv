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
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) FString Name;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) UTexture* Portrait;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) FString Description;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) int32 TeamId;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) int32 StoneCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float RepairRate;
  // Generic time: 
  //   Units: For units, its the training time
  //   Items: Time length item lasts for
  //   Spells: Duration of the spell
  //   Buildings: Time to build
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float TimeLength;
  // Time it takes for object to be re-used. Only applies to Items & Spells.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float Cooldown;
  // Generic Property:
  //   * Items: Application interval
  //   * Spells: (continuous effect) Damage interval
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float Interval;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float SightRange;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float SpeedMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float HpMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float Armor;
  // How many seconds per HP recovered, when Repairing
  // Attack damage is BaseAttackDamage + rand()%BonusAttackDamage
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) int32 BaseAttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) int32 BonusAttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float AttackSpeedMultiply;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float MissPercent;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float FollowFallbackDistance;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float PickupRange;
  // Range it can attack from (spells have attack range also)
  //   * For a Spell, its the range of the spell itself.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) float AttackRange;
  // The range this unit can pick things up from.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) bool Invulnerability;
  // How much the unit uses
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats) int32 FoodUsed;
  // Construction requirements: things to have built or possess to be able to build this object.
  // Usually these are lists of building classes you must have.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
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
    TimeLength = 2.f;
    Cooldown = 10.f;
    Interval = 1.f;
    
    SightRange = 500.f;
    SpeedMax = 100.f;
    HpMax = 100.f;
    Armor = 1.f;

    BaseAttackDamage = 1;
    BonusAttackDamage = 1;
    AttackSpeedMultiply = 1.f;
    MissPercent = 0.2f;
    
    FollowFallbackDistance = 250.f;
    PickupRange = 50.f;
    AttackRange = 50.f;
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
    copy.TimeLength += o.TimeLength;
    copy.Cooldown   += o.Cooldown;
    copy.Interval   += o.Interval;
    
    copy.SightRange += o.SightRange;
    copy.SpeedMax   += o.SpeedMax;
    copy.HpMax      += o.HpMax;
    copy.Armor      += o.Armor;
    
    copy.BaseAttackDamage  += o.BaseAttackDamage;
    copy.BonusAttackDamage += o.BonusAttackDamage;
    copy.AttackSpeedMultiply += o.AttackSpeedMultiply;
    copy.MissPercent += o.MissPercent;
    
    copy.FollowFallbackDistance += o.FollowFallbackDistance;
    copy.PickupRange += o.PickupRange;
    copy.AttackRange += o.AttackRange;
    copy.Invulnerability |= o.Invulnerability;
    copy.FoodUsed    += o.FoodUsed;
    return copy;
  }
};


