#pragma once

#include "RTSGame.h"
#include "Types.h"
#include "ItemData.generated.h"

USTRUCT()
struct RTSGAME_API FItemDataRow : public FTableRowBase
{
  GENERATED_USTRUCT_BODY()
public:
  // The UnitTYPE (strict set of types)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MagicData) FString Name;

  // Attack and defense properties
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MagicData) int32 Speed;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MagicData) int32 AttackDamage;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MagicData) int32 AttackCooldown;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MagicData) int32 AttackRange;

  // What this unit type spawns
  // Type of unit spawned when this widget is pressed.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MagicData)
  TArray< TEnumAsByte<Types> > Spawns;

  FItemDataRow()
  {
    
  }

};
