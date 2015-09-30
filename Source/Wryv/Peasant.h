#pragma once

#include <map>
using namespace std;

#include "Unit.h"
#include "Peasant.generated.h"

class ABuilding;
class AResource;

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

  // The unit is carrying something
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  bool Carrying;
  // Plays the shrug animation
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Stats )  bool Shrugging;

  // These are set when the peasant is repairing a building or unit.
  ABuilding* RepairTarget;  // The building we are repairing.
  FVector LastResourcePosition;
  TSubclassOf<AResource> Mining;   // The type of resource that I'm mining
  
  // [The resource type I'm mining] => The amount of resource I'm holding.
  map< TSubclassOf<AResource>, int32 >  MinedResources;
  map< TSubclassOf<AResource>, UStaticMeshComponent* >  MinedPieces;
  map< TSubclassOf<AResource>, int32 >  Capacities;  // How much of each resource this unit can carry
  
  //APeasant(const FObjectInitializer& PCIP);
  void PostInitializeComponents();
  bool Build( UBuildAction* buildAction, FVector pos );
  virtual void Target( AGameObject* target ) override;
  virtual void DropTargets() override;
  void Repair( float t );
  AResource* FindAndTargetNewResource( FVector fromPos, vector< TSubclassOf<AResource> > types, float searchRadius );
  UFUNCTION(BlueprintCallable, Category = Fighting)  virtual void AttackCycle();
  // Is the unit repairing something, so play the repair animation
  UFUNCTION( BlueprintCallable, Category = Stats )  bool IsRepairing();
  AGameObject* GetBuildingMostInNeedOfRepair( float threshold );
  void ReturnResources();
  void AddMined( TSubclassOf<AResource> resourceType, float resAmount );
  virtual void Move( float t );
  virtual bool Idling();
  virtual void ai( float t );
  void OnResourcesReturned();
  virtual void Hit( AGameObject* other );
  // We are the primary builder if the building is set to have it as primary
  void JobDone();
  virtual void Die();
};
