#include "Wryv.h"
#include "Resource.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

AResource::AResource( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
  // Make sure that the object type is actually a resource type
  if( Stats.Type != Types::RESGOLDMINE || Stats.Type != Types::RESTREE || Stats.Type != Types::RESSTONE )
  {
    //LOG(  "Object %s was not a resource", *Stats.Name );
  }

  Stats.Team = 0;
}

void AResource::BeginPlay()
{
  Super::BeginPlay();

  // Get the team 0, which resources automatically belong to
  team = Game->gm->teams[0];
}

