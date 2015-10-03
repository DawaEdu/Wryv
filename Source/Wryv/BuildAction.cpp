#include "Wryv.h"

#include "Building.h"
#include "BuildAction.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "WryvGameInstance.h"

UBuildAction::UBuildAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UBuildAction::Click(APeasant* peasant)
{
  // flycam places ghost of building to build next.
  Game->flycam->ghost = Game->Make< ABuilding >( BuildingType );
}


