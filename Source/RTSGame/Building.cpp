#include "RTSGame.h"
#include "Building.h"

ABuilding::ABuilding( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
  buildProgress = 0.f;
}

void ABuilding::Tick( float t )
{
  buildProgress += t;
  if( buildProgress < UnitsData.BuildTime )
  {
    // Building not complete yet, so increase HP and
    float perc = t / UnitsData.BuildTime;

    // increase HP by that fraction
    hp += perc * UnitsData.HpMax;
    // The building cannot do anything else while building.
    //return;
  }
}

