#pragma once

#include <map>
using namespace std;

#include "Unit.h"
#include "Peasant.generated.h"

class AResource;
class ABuilding;

UCLASS()
class WRYV_API APeasant : public AUnit
{
  GENERATED_UCLASS_BODY()
public:
  float MiningTime;  // Total time the peasant has been mining for
  ABuilding* building; // The building this unit is building
  AGameObject* repair; // The building this unit is currently repairing
  AResource* mining; // The Resource we are mining

  //APeasant(const FObjectInitializer& PCIP);
  virtual void BeginPlay() override;
  AGameObject* Build( Types type, FVector location );
  bool PlaceBuilding( ABuilding* go, FVector location );
  AGameObject* PlaceBuildingAtRandomLocation( Types type );
  void Build( float t );
  void Repair( float t );
  void Mine( float t );
  AGameObject* GetBuildingMostInNeedOfRepair( float threshold );
  virtual void SetTarget( AGameObject* go );
  virtual void Move( float t );
  virtual void ai( float t );
  bool isBusy(){ return building!=NULL || attackTarget!=NULL; }
  
};
