#pragma once

#include <vector>
using namespace std;

#include "Types.h"
#include "GameFramework/Actor.h"
#include "GameObject.h"
#include "Unit.generated.h"

// Generate a series of blueprints from this same Unit class.

UCLASS()
class RTSGAME_API AUnit : public AGameObject
{
	GENERATED_UCLASS_BODY()
public:
  // If the unit HAS a projectile, then its attack is not melee and
  // the range it can attack from is "weaponRange"
  // The types of special attacks this hero has.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=UnitProperties)
	TArray< TEnumAsByte<Types> > Spells;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=UnitProperties)
	TArray< TEnumAsByte<Types> > Items;

  // list of points to visit to finally reach destination
  vector<FVector> waypoints;
  
  //AUnit(const FObjectInitializer& PCIP);
  
};
