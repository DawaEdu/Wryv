#pragma once

#include "GameObject.h"
#include "Building.generated.h"

class APeasant;
class UParticleEmitter;
class UInProgressUnit;
class UInProgressResearch;
class AUnit;

UCLASS()
class WRYV_API ABuilding : public AGameObject
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cosmetics)  USkeletalMeshComponent* Mesh;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cosmetics)  USceneComponent* ExitPosition;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cosmetics)  UDestructibleComponent* destructableMesh;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cosmetics)  UParticleSystemComponent* buildingDust;
  // The MONTAGE for building the building. Montages must be used to call SetPosition() on.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cosmetics)  UAnimMontage* buildingMontage;

  // Repair costs a fraction of GoldCost, LumberCost, StoneCost per HP recovered.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)  float RepairHPFractionCost;
  
  // Time to wait after exploding for cleanup
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)  float MaxExplosionTime;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)  float ExplosiveRadius;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)  float ExplosiveForce;

  // Food this structure supplies (Farms + townhall)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)  int32 FoodProvided;
  
  // List of types this building can train.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TArray< TSubclassOf< UTrainingAction > > TrainClasses;

  // Researches that this Building can do
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TArray< TSubclassOf< UResearch > > ResearchClasses;
  
  UPROPERTY() TArray< UTrainingAction* > TrainingAvailable;
  UPROPERTY() TArray< UInProgressUnit* > CountersUnitsInProgress;
  UPROPERTY() TArray< UResearch* > ResearchesAvailable;  // Research kick-off buttons.
  UPROPERTY() TArray< UInProgressResearch* > CountersResearchInProgress; // Cancellable, started researches.

  APeasant* PrimaryPeasant;      // The main peasant creating the building ( doesn't use resource to build )
  float TimeBuilding;     // When a unit is building, this is the % progress it is to completion.
  bool Complete;          // Set to true when the building is complete.
  float ExplodedTime;     // The length of time the building has been exploded for
  
  ABuilding(const FObjectInitializer& PCIP);
  virtual void PostInitializeComponents() override;
  virtual void BeginPlay() override;
  void InitIcons();
  void LosePeasant( APeasant* peasant );
  virtual void Move( float t ) override;
  virtual void Tick( float t ) override;

  // Counters & Training
  virtual void MoveCounters( float t ) override;
  bool UseTrain( int index );
  bool CancelTraining( int index );
  bool UseResearch( int index );
  bool CancelResearch( int index );
  virtual void OnUnselected();

  // Tell you if building can be placed @ Pos
  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void montageStarted( UAnimMontage* Montage );
  bool CanBePlaced();
  void PlaceBuilding( APeasant *p );
  void ReleaseUnit( UInProgressUnit* unit );
  void DropBuilders( bool buildingSuccess );
  void OnBuildingComplete();
  FVector GetExitPosition() { return ExitPosition->GetComponentLocation(); }
  UFUNCTION(BlueprintCallable, Category = Building)  float PercentBuilt() { return TimeBuilding / Stats.TimeLength; }
  UFUNCTION(BlueprintCallable, Category = Building)  float BuildTime() { return Stats.TimeLength; }
  UFUNCTION(BlueprintCallable, Category = Building)  bool BuildingDone() { return TimeBuilding >= Stats.TimeLength; }
  // Tells you if the building needs a peasant or not.
  UFUNCTION(BlueprintCallable, Category = Building)  bool NeedsPeasant() {
    return !PrimaryPeasant && !Complete;
  }
  // The HP added for t seconds of building.
  float GetHPAdd( float t ) { return t * Stats.HpMax / Stats.TimeLength; }
  void Cancel();
  virtual void Die();
};
