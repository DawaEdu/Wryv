#include "Wryv.h"
#include "FogOfWar.h"
#include "FlyCam.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

AFogOfWar::AFogOfWar( const FObjectInitializer& PCIP )
{
}

// we'll start with the fogofwar covering the entire scene.
// then minimize its size to increase resolution
void AFogOfWar::BeginPlay()
{
  // Grab the mesh and set the scale of the basemesh component
  // (just a single square) to expand across the floor terrain object
  vector<UMeshComponent*> meshComponents = GetComponentsByType<UMeshComponent>( this );
  if( meshComponents.size() )
  {
    MeshComponent = meshComponents[0];
    MeshComponent->SetWorldScale3D( Game->flycam->floorBox.GetSize() );
  }
  else
  {
    LOG( "Couldn't find mesh component in FogOfWar");
  }
  SetActorLocation( FVector( 0, 0, 1500 ) );
}

