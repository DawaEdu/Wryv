#include "Wryv.h"

#include "Building.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "UnitsData.h"
#include "WryvGameInstance.h"

#include "InProgressUnit.h"
#include "TrainingAction.h"

UInProgressUnit::UInProgressUnit( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

UTexture* UInProgressUnit::GetIcon()
{
  return Game->GetData( UnitType ).Portrait;
}

float UInProgressUnit::GetCooldownTotalTime()
{
  return Game->GetData( UnitType ).Cooldown;
}

void UInProgressUnit::Set( UTrainingAction* training )
{
  UnitType = training->UnitType; // Class of the unit that is being created
  HostBuilding = training->Building; // Cancellation req goes thru to building owning icon
}

bool UInProgressUnit::Click()
{
  // returns money for training the unit
  if( !HostBuilding )
  {
    error( "Building not set" );
    return 0;
  }

  HostBuilding->team->Refund( UnitType );
  return 1;
}

void UInProgressUnit::OnCooldown()
{
  UAction::OnCooldown();
  HostBuilding->ReleaseUnit( UnitType );
}


