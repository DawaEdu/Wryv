#pragma once

#include "GameObject.h"
#include "GameFramework/Actor.h"
#include "FogOfWar.generated.h"

UCLASS()
class WRYV_API AFogOfWar : public AActor
{
	GENERATED_UCLASS_BODY()
public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD )  UMeshComponent* MeshComponent;
  
  //AFogOfWar( const FObjectInitializer& PCIP );
  virtual void BeginPlay();
};
