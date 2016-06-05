#pragma once

#include <vector>
using namespace std;

#include "Game/Enums.h"
#include "Util/Cost.h"
#include "AI.generated.h"

struct Team;
class AResource;

// The artificial intelligence for the team. Parameters for the AI.
USTRUCT()
struct WRYV_API FAI
{
  GENERATED_USTRUCT_BODY()

  // The AI level being used. 0 is no ai (for player-controlled)
  // higher levels mean higher ai (greater difficulty)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) TEnumAsByte<AILevel> aiLevel;
  // How much spare food do we want to keep in stock?
  // If food% is 40/60 (66%) then perhaps we want to build another farm.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) float foodFraction;
  // The fraction at which the hp of a building needs repair.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) float repairFraction;
  // Attacks are periodic. We scout at intervals.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI) float scoutInterval;
  float timeSinceLastScout;
  // Need some defaults for stock Gold, Lumber, Stone AI level expects to have
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
  FCost StartingResources;
  
  FAI();
  vector< TSubclassOf<AResource> > GetNeededResourceTypes( Team& team );
  FString ToString();
};

