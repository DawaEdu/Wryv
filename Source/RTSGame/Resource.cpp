#include "RTSGame.h"
#include "Resource.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"

AResource::AResource( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
  // Make sure that the object type is actually a resource type
  if( UnitsData.Type != Types::RESGOLDMINE || UnitsData.Type != Types::RESTREE || UnitsData.Type != Types::RESSTONE )
  {
    //UE_LOG( LogTemp, Warning, TEXT( "Object %s was not a resource" ), *UnitsData.Name );
  }

  UnitsData.Team = 0;
}

void AResource::BeginPlay()
{
  Super::BeginPlay();

  // Get the team 0, which resources automatically belong to
  team = Game->gm->teams[0];
}

