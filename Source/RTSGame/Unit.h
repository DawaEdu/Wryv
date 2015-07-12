#pragma once

#include <vector>
using namespace std;

#include "Types.h"
#include "GameFramework/Actor.h"
#include "GameObject.h"
#include "Unit.generated.h"

struct PowerUpTimeOut
{
  float time;
  FUnitsDataRow traits;
  PowerUpTimeOut():time(0.f){}
  PowerUpTimeOut( float t, FUnitsDataRow &iTraits ) :
    time( t ), traits( iTraits ) { }
  void Tick( float t ) { time += t; }
};

UCLASS()
class RTSGAME_API AUnit : public AGameObject
{
	GENERATED_UCLASS_BODY()
public:
  // These are the units capabilities.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=UnitProperties)
	TArray< TEnumAsByte<Types> > Spells;

  // These start with # items in editor. Defaults "Nothing" in toolbelt
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=UnitProperties)
	TArray< TEnumAsByte<Types> > Items;

  // list of points to visit to finally reach destination
  vector<FVector> waypoints;

  // List set of traits that gets applied due to powerups
  //   0.025 => FUnitsDataRow(),  0.150 => FUnitsDataRow(),  0.257 => FUnitsDataRow()
  vector< PowerUpTimeOut > BonusTraits;
  
  //AUnit(const FObjectInitializer& PCIP);
  void ApplyEffect( Types type );
  
  // Function that runs whenever the unit is first clicked on or selected.
  virtual void OnSelected() override;

  FUnitsDataRow GetTraits();

  virtual void Tick( float t ) override;
};
