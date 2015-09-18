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
  // How much the mining target can hold.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 Capacity;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  USoundBase* JobsDoneSound;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  USceneComponent* ResourceCarry;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  UStaticMeshComponent* GoldPiece;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  UStaticMeshComponent* LumberPiece;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  UStaticMeshComponent* StonePiece;
  
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 GoldCarryCapacity;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 LumberCarryCapacity;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  int32 StoneCarryCapacity;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  bool Carrying;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  bool Shrugging;
  ABuilding* Building;
  //ABuilding* RepairTarget;
  FVector LastResourcePosition;
  Types Mining;   // The type of resource that I'm mining
  bool Repairing; // Is the unit repairing something

  // [The resource type I'm mining] => The amount of resource I'm holding.
  map< Types, int32 >  MinedResources;
  map< Types, UStaticMeshComponent* >  MinedPieces;
  map< Types, int32 >  Capacities;  // How much of each resource this unit can carry
  
  //APeasant(const FObjectInitializer& PCIP);
  void PostInitializeComponents();
  void Build( Types type, FVector pos );
  virtual void Target( AGameObject* target ) override;
  void Repair( float t );
  AResource* FindNewResource( FVector fromPos, Types type, float searchRadius );
  UFUNCTION(BlueprintCallable, Category = Fighting)  virtual void AttackCycle();
  AGameObject* GetBuildingMostInNeedOfRepair( float threshold );
  void ReturnResources();
  virtual void Move( float t );
  virtual void ai( float t );
  void OnResourcesReturned();
  virtual void Hit( AGameObject* other );
  bool isBusy(){ return AttackTarget || FollowTarget || Waypoints.size(); }
  void JobDone();
};
