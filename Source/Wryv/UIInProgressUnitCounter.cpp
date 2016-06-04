#include "Wryv.h"

#include "Building.h"
#include "GlobalFunctions.h"
#include "UnitsData.h"
#include "WryvGameInstance.h"

#include "UIInProgressUnitCounter.h"
#include "UITrainingActionCommand.h"

UUIInProgressUnitCounter::UUIInProgressUnitCounter( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  UnitType = 0;
}

UTexture* UUIInProgressUnitCounter::GetIcon()
{
  return Game->GetData( UnitType ).Portrait;
}

float UUIInProgressUnitCounter::GetCooldownTotalTime()
{
  return Game->GetData( UnitType ).Cooldown;
}

void UUIInProgressUnitCounter::Set( UUITrainingActionCommand* training )
{
  UnitType = training->UnitType; // Class of the unit that is being created
  HostBuilding = training->Building; // Cancellation req goes thru to building owning icon
  cooldown = training->cooldown;
}

bool UUIInProgressUnitCounter::Click()
{
  // returns money for training the unit
  if( !HostBuilding )
  {
    error( "Building not set" );
    return 0;
  }

  HostBuilding->CancelTraining( UUICmdActionIndex );
  return 1;
}

void UUIInProgressUnitCounter::PopulateClock( Clock* inClock, int index )
{
  UUIActionCommand::PopulateClock( inClock, index );
}

void UUIInProgressUnitCounter::OnCooldown()
{
  UUIActionCommand::OnCooldown();
  HostBuilding->ReleaseUnit( this );
}


