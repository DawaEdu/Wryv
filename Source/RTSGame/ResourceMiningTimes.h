#pragma once

#include "RTSGame.h"
#include "Types.h"
#include "ResourceMiningTimes.generated.h"

USTRUCT()
struct RTSGAME_API FResourceMiningTimes
{
  GENERATED_USTRUCT_BODY()
public:
  // ResourceType
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TEnumAsByte<Types> Type;

  // Time to mine that resource
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  float Time;
};
