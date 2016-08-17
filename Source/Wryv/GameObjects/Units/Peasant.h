#pragma once

#include <map>
using namespace std;

#include "Util/Cost.h"
#include "GameObjects/Units/Unit.h"
#include "Peasant.generated.h"

class ABuilding;
class AResource;

UCLASS()
class WRYV_API APeasant : public AUnit
{
  GENERATED_BODY()
public:
  // How much the mining target can hold.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Peasant )  USoundBase* JobsDoneSound;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Peasant )  USceneComponent* ResourceCarry;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Peasant )  UStaticMeshComponent* GoldPiece;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Peasant )  UStaticMeshComponent* LumberPiece;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Peasant )  UStaticMeshComponent* StonePiece;
  
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Peasant )  FCost CarryCapacities;
  // The rate at which a resource is gathered
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Peasant )  float GatheringRate;
  // Plays the shrug animation
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Peasant )  bool Shrugging;
  bool ShrugsNextIdle;

  // Buildings we can build using this unit
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TArray< TSubclassOf< UUIBuildActionCommand > > Builds;

  // List of objects that are currently being built by this object.
  UPROPERTY() TArray< UUIBuildActionCommand* > Buildables; // Buildings themselves
  // Buildings that this peasant is constructing. This will contain a queue
  // of things in case the peasant has queued several buildings.
  UPROPERTY() TArray< UUIInProgressBuilding* > CountersBuildingsQueue;

  // These are set when the peasant is repairing a building or unit.
  ABuilding* RepairTarget;  // The building we are repairing.
  FVector LastResourcePosition;
  TSubclassOf<AResource> Mining;   // The type of resource that I'm mining
  
  // [The resource type I'm mining] => The amount of resource I'm holding.
  map< TSubclassOf<AResource>, int32 >  MinedResources;
  map< TSubclassOf<AResource>, UStaticMeshComponent* >  MinedPieces;
  map< TSubclassOf<AResource>, int32 >  Capacities;  // How much of each resource this unit can carry
  
  APeasant(const FObjectInitializer& PCIP);
  void PostInitializeComponents();
  virtual void InitIcons();
  bool UseBuild( int index );
  bool CancelBuilding( int index );

  bool Build( TSubclassOf<ABuilding> BuildingClass, FVector position );
  virtual void Target( AGameObject* target ) override;
  virtual void DropTargets() override;
  void Repair( float t );
  AResource* FindAndTargetNewResource( FVector fromPos, vector< TSubclassOf<AResource> > types, float searchRadius );
  UFUNCTION( BlueprintCallable, Category = Fighting )  virtual void AttackCycle();
  // Is the unit repairing something, so play the repair animation
  UFUNCTION( BlueprintCallable, Category = Stats )  bool IsRepairing();
  // The unit is carrying something
  UFUNCTION( BlueprintCallable, Category = Stats )  bool IsCarrying();
  
  AGameObject* GetBuildingMostInNeedOfRepair( float threshold );
  void ReturnResources();
  void AddMined( TSubclassOf<AResource> resourceType, float resAmount );
  virtual void MoveCounters( float t );
  virtual void Move( float t );
  virtual bool Idling();
  virtual void ai( float t );
  void OnResourcesReturned();
  virtual void Hit( AGameObject* other );
  // We are the primary builder if the building is set to have it as primary
  void JobDone();
  virtual void Die();
};
