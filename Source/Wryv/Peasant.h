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
  Types MinedResourceType;  // The resource type I'm mining
  int32 MinedQuantity;    // The amount of resource I'm holding.

  //APeasant(const FObjectInitializer& PCIP);
  virtual void BeginPlay() override;
  virtual void Target( AGameObject* target ) override;
  void Repair( float t );
  UFUNCTION(BlueprintCallable, Category = Mining)
  void Mine( float t );
  AGameObject* GetBuildingMostInNeedOfRepair( float threshold );
  virtual void Move( float t );
  virtual void ai( float t );
  bool isBusy(){ return AttackTarget || FollowTarget || Waypoints.size(); }
  
};
