#pragma once

#include "RTSGame.h"
#include "Types.h"
#include "UnitsData.generated.h"

USTRUCT()
struct RTSGAME_API FUnitsDataRow : public FTableRowBase
{
  GENERATED_USTRUCT_BODY()
public:
  // The UnitTYPE (strict set of types)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TEnumAsByte<Types> Type;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Name;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) FString Description;

  // Build cost
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 GoldCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 LumberCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 StoneCost;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 ManaCost;
  // Repair costs a fraction of GoldCost, LumberCost, StoneCost per HP recovered.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairHPFractionCost;
  // How many seconds per HP recovered, when repairing
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float RepairRate;
  // Time it takes to build this thing.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 BuildTime;
  
  // Attack and defense properties
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Speed;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 HpMax;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Armor;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 SightRange;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 AttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 AttackCooldown;
  // Range it can attack from (spells have attack range also)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 AttackRange;
  // Team it belongs to
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Team;
  // Food this structure supplies (Farms + townhall)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 FoodProvided;
  /// How much the unit or building uses
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 FoodUsed;
  // Length of the effect (only used for time-limited things such as spells or items)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float TimeLength;

  // What this unit type spawns
  // Type of unit spawned when this widget is pressed.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< TEnumAsByte<Types> > Spawns;

  // The blueprint from which class instance came from
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) UClass* uClass;

  FUnitsDataRow()
  {
    Type = NOTHING;
    GoldCost = LumberCost = StoneCost = BuildTime = Speed = HpMax = Armor = SightRange = 
      AttackDamage = AttackCooldown = AttackRange = Team = FoodProvided = FoodUsed = 0;
    TimeLength = 0.f;
    uClass = 0;
  }

  FUnitsDataRow operator+=( const FUnitsDataRow& row )
  {
    FUnitsDataRow r;
    r.GoldCost += row.GoldCost;
    r.LumberCost += row.LumberCost;
    r.StoneCost += row.StoneCost;
    r.BuildTime += row.BuildTime;
    r.Speed += row.Speed;
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
      Speed, HpMax, Armor, SightRange, AttackDamage, AttackCooldown, AttackRange );
    return fs;
  }

};
