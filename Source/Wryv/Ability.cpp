#include "Wryv.h"
#include "Ability.h"
#include "WryvGameInstance.h"

Ability::Ability( Types type ) : 
  CooldownCounter( type ), Enabled( 1 )
{
  TotalTime = Game->unitsData[ type ].TimeLength;
}

CooldownCounter::CooldownCounter( Types type ) :
  Type(type), Time(0.f)
{
  TotalTime = Game->unitsData[ type ].TimeLength;
}
  
