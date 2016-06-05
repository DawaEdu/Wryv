#include "Wryv.h"

#include "GameObjects/Buildings/Building.h"
#include "Util/GlobalFunctions.h"
#include "Game/UnitsData.h"
#include "UE4/WryvGameInstance.h"

#include "UI/UICommand/Counters/UIInProgressUnitCounter.h"
#include "UI/UICommand/Command/UITrainingActionCommand.h"

UUIInProgressUnit::UUIInProgressUnit( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  UnitType = 0;
}

UTexture* UUIInProgressUnit::GetIcon()
{
  return Game->GetData( UnitType ).Portrait;
}

float UUIInProgressUnit::GetCooldownTotalTime()
{
  return Game->GetData( UnitType ).Cooldown;
}

void UUIInProgressUnit::Set( UUITrainingActionCommand* training )
{
  UnitType = training->UnitType; // Class of the unit that is being created
  HostBuilding = training->Building; // Cancellation req goes thru to building owning icon
  cooldown = training->cooldown;
}

bool UUIInProgressUnit::Click()
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

void UUIInProgressUnit::PopulateClock( Clock* inClock, int index )
{
  UUIActionCommand::PopulateClock( inClock, index );
}

void UUIInProgressUnit::OnCooldown()
{
  UUIActionCommand::OnCooldown();
  HostBuilding->ReleaseUnit( this );
}


