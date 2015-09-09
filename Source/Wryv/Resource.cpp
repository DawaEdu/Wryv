#include "Wryv.h"
#include "Resource.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "FlyCam.h"

AResource::AResource( const FObjectInitializer& PCIP ) : AGameObject( PCIP )
{
  StaticMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "staticmesh1" );
  StaticMesh->AttachTo( RootComponent );
}

void AResource::BeginPlay()
{
  Super::BeginPlay();
  
  
}


