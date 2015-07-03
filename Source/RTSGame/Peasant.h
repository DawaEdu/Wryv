#pragma once

#include <map>
using namespace std;

#include "Unit.h"
#include "ResourceMiningTimes.h"
#include "Peasant.generated.h"

class AResource;

UCLASS()
class RTSGAME_API APeasant : public AUnit
{
  GENERATED_UCLASS_BODY()
public:
  // The amount of time each resource
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TArray< FResourceMiningTimes > ResourceTypes;

  // This is the mine time for each type of resource, by resource type
  map< int, float > ResourceMineTimes;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  float MiningDistance;
  float MiningTime;  // Total time the peasant has been mining for
  AGameObject* building; // The building this unit is building
  AGameObject* repair; // The building this unit is currently repairing
  AResource* mining; // The Resource we are mining

  //APeasant(const FObjectInitializer& PCIP);
  virtual void BeginPlay() override;
  AGameObject* Build( Types type, FVector location );
  bool PlaceBuilding( AGameObject* go, FVector location );
  AGameObject* PlaceBuildingAtRandomLocation( Types type );
  void Build( float t );
  void Repair( float t );
  void Mine( float t );
  AGameObject* GetBuildingMostInNeedOfRepair( float threshold );
  virtual void SetTarget( AGameObject* go );
  virtual void ai( float t );
  bool isBusy(){ return building!=NULL || attackTarget!=NULL; }
  
};
