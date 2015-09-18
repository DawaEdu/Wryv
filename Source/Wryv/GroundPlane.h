#pragma once

#include "GameObject.h"
#include "GameFramework/Actor.h"
#include "GroundPlane.generated.h"

UCLASS()
class WRYV_API AGroundPlane : public AGameObject
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  UStaticMeshComponent* Mesh;
  UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Pathfinding )  FBox Box;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  float TraceFraction;
  
  virtual void PostInitializeComponents() override;
  FBox GetReducedBox();
};
