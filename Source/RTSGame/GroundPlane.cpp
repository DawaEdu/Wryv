#include "RTSGame.h"
#include "GroundPlane.h"
#include "GameObject.h"
#include "GlobalFunctions.h"

// Sets default values
AGroundPlane::AGroundPlane( const FObjectInitializer& PCIP ) : Super( PCIP )
{
 	// Set this actor to call Tick() every frame.
  // You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

FVector AGroundPlane::getRandomLocation()
{
  FBox box = GetComponentsBoundingBox();
  FVector v = Rand( box.Min, box.Max );
  v.Z = box.Max.Z; // keep location on ground plane
  return v;
}


