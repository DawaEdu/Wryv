#include "Wryv.h"

#include "Building.h"
#include "InProgressBuilding.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Peasant.h"

UInProgressBuilding::UInProgressBuilding( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Building = 0;
  Peasant = 0;
}

void UInProgressBuilding::SetBuilding(ABuilding* building)
{
  Building = building;
  cooldown.TotalTime = Building->BuildingTime;
}

void UInProgressBuilding::Cancel()
{
  if( Building )
  {
    Building->Cancel();
    removeElement( Peasant->CountersBuildingsQueue, this );
  }
}

void UInProgressBuilding::OnComplete()
{
  UAction::OnComplete();
}

void UInProgressBuilding::Step( float t )
{
  // The step function pulls from the building in progress
  cooldown.Time = Building->TimeBuilding;
}