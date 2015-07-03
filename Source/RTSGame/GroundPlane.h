#pragma once

#include "GameObject.h"
#include "GameFramework/Actor.h"
#include "GroundPlane.generated.h"

UCLASS()
class RTSGAME_API AGroundPlane : public AGameObject
{
	GENERATED_UCLASS_BODY()
public:	
	// Sets default values for this actor's properties
	//AGroundPlane( const FObjectInitializer& PCIP );
  FVector getRandomLocation();
};
