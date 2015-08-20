#include "Wryv.h"
#include "Building.h"
#include "GlobalFunctions.h"

ABuilding::ABuilding( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
  TimeBuilding = 0.f;
}

void ABuilding::Move( float t )
{
  AGameObject::Move( t );

  TimeBuilding += t;
  if( TimeBuilding < Stats.TimeLength )
  {
    // Building not complete yet, so increase HP and increase HP by a fraction
    Hp += t / Stats.TimeLength * Stats.HpMax;// hp increases because building may be attacked while being built.
    // The building cannot do anything else while building.
    Clamp( Hp, 0.f, Stats.HpMax );
  }
  else
  {
    // Building complete.

  }
}


