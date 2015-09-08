#pragma once

#include "GameObject.h"
#include "Building.generated.h"

class APeasant;
class UParticleEmitter;

UCLASS()
class WRYV_API ABuilding : public AGameObject
{
	GENERATED_UCLASS_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  USceneComponent* ExitPosition;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  UDestructibleComponent* destructableMesh;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  UParticleSystemComponent* buildingDust;
  // Time to wait after exploding for cleanup
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  float MaxExplosionTime;
  
  //ABuilding(const FObjectInitializer& PCIP);
  APeasant* peasant;      // The peasant building, inside the building.
  float TimeBuilding;     // When a unit is building, this is the % progress it is to completion.
  bool CanBePlaced;       // Set to true when building is hovering over a position it can be placed in.
  bool Complete;          // Set to true when the building is complete.
  float ExplodedTime;     // The length of time the building has been exploded for
  virtual void BeginPlay() override;
  virtual void Move( float t ) override;
  virtual void Tick( float t ) override;
  void SetPeasant( APeasant *p );
  void OnBuildingComplete();
  FVector GetExitPosition();
  UFUNCTION(BlueprintCallable, Category = Building)  float PercentBuilt() { return TimeBuilding / Stats.TimeLength; }
  UFUNCTION(BlueprintCallable, Category = Building)  float BuildTime() { return Stats.TimeLength; }
  UFUNCTION(BlueprintCallable, Category = Building)  bool BuildingDone() { return TimeBuilding >= Stats.TimeLength; }
  UFUNCTION(BlueprintNativeEvent, Category = Building)
  void BuildingExploded();
  virtual void Die();
};
