#pragma once

#include "GameObjects/GameObject.h"
#include "GameFramework/Actor.h"
#include "GroundPlane.generated.h"

UCLASS()
class WRYV_API AGroundPlane : public AGameObject
{
  GENERATED_BODY()
public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  UStaticMeshComponent* Mesh;
  //UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Pathfinding )  FBox Box;
  //UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Pathfinding )  FBox ReducedBox;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  float TraceFraction;
  UFUNCTION( BlueprintCallable, Category = Fighting )  FBox GetBox();
  UFUNCTION( BlueprintCallable, Category = Fighting )  FBox GetReducedBox();

  AGroundPlane( const FObjectInitializer& PCIP );
  virtual void PostInitializeComponents() override;
  
};
