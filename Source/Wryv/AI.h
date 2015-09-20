#pragma once

#include "Wryv.h"
#include "Types.h"
#include "AI.generated.h"

struct Team;

UENUM()
enum AILevel
{
  AINone    UMETA(DisplayName = "AINone"),
  AIEasy    UMETA(DisplayName = "AIEasy"),
  AIMedium  UMETA(DisplayName = "AIMedium"),
  AIHard    UMETA(DisplayName = "AIHard")
};

// The artificial intelligence for the team. Parameters for the AI.
USTRUCT() //( BlueprintType, Blueprintable, meta=(ShortTooltip="An AI profile") )
struct WRYV_API FAI
{
  GENERATED_USTRUCT_BODY()

  // The AI level being used. 0 is no ai (for player-controlled)
  // higher levels mean higher ai (greater difficulty)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) TEnumAsByte<AILevel> aiLevel;
  // How much spare food do we want to keep in stock?
  // If food% is 40/60 (66%) then perhaps we want to build another farm.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) float foodFraction;
  // Attacks are periodic. We scout at intervals.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) float scoutInterval;
  float timeSinceLastScout;
  // Need some defaults for stock Gold, Lumber, Stone AI level expects to have
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) int32 StartingGold;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) int32 StartingLumber;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) int32 StartingStone;
  //FAI()
  //{
  //  //aiLevel = AILevel::AINone;
  //  foodFraction = 2.f/3.f; // default of 66%
  //  timeSinceLastScout = 0.f;
  //  scoutInterval = 20.f;
  //  // Try to keep these stock amounts of each resource type
  //  StartingGold = 1000;
  //  StartingLumber = 500;
  //  StartingStone = 50;
  //}
  //Types GetNeededResourceType( Team& team );
  FString ToString()
  {
    return FS( "AILevel aiLevel=%d foodFraction=%f scoutInterval=%f gold=%d lumber=%d stone=%d", 
      (int)aiLevel.GetValue(), foodFraction, scoutInterval,
      StartingGold, StartingLumber, StartingStone );
  }
};

