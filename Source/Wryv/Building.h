#pragma once

#include "GameObject.h"
#include "Building.generated.h"

class APeasant;
class UParticleEmitter;

UCLASS()
class WRYV_API ABuilding : public AGameObject
{
	GENERATED_UCLASS_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  USkeletalMeshComponent* Mesh;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  USceneComponent* ExitPosition;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  UDestructibleComponent* destructableMesh;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  UParticleSystemComponent* buildingDust;
  // The MONTAGE for building the building. Montages must be used to call SetPosition() on.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  UAnimMontage* buildingMontage;

  // Time to wait after exploding for cleanup
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  float MaxExplosionTime;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  float ExplosiveRadius;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  float ExplosiveForce;

  // The researches this building is able to do
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)
  TArray< UResearch* > Researches;

  //ABuilding(const FObjectInitializer& PCIP);
  APeasant* PrimaryPeasant;      // The main peasant creating the building ( doesn't use resource to build )
  float TimeBuilding;     // When a unit is building, this is the % progress it is to completion.
  bool Complete;          // Set to true when the building is complete.
  float ExplodedTime;     // The length of time the building has been exploded for
  virtual void PostInitializeComponents() override;
  virtual void BeginPlay() override;
  void LosePeasant( APeasant* peasant );
  virtual void Move( float t ) override;
  virtual void Tick( float t ) override;
  // Tell you if building can be placed @ Pos
  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void montageStarted( UAnimMontage* Montage );
  bool CanBePlaced();
  void PlaceBuilding( APeasant *p );
  void OnBuildingComplete();
  FVector GetExitPosition();
  UFUNCTION(BlueprintCallable, Category = Building)  float PercentBuilt() { return TimeBuilding / Stats.TimeLength; }
  UFUNCTION(BlueprintCallable, Category = Building)  float BuildTime() { return Stats.TimeLength; }
  UFUNCTION(BlueprintCallable, Category = Building)  bool BuildingDone() { return TimeBuilding >= Stats.TimeLength; }
  // Tells you if the building needs a peasant or not.
  UFUNCTION(BlueprintCallable, Category = Building)  bool NeedsPeasant() {
    return !PrimaryPeasant && !Complete;
  }
  // The HP added
  float GetHPAdd( float t ) { return t * Stats.HpMax / Stats.TimeLength; }
  virtual void Die();
};
