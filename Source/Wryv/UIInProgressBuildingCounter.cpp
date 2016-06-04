#include "Wryv.h"

#include "Building.h"
#include "Peasant.h"
#include "GlobalFunctions.h"
#include "UIInProgressBuildingCounter.h"

UUIInProgressBuildingCounter::UUIInProgressBuildingCounter( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Building = 0;
  Peasant = 0;
}

UTexture* UUIInProgressBuildingCounter::GetIcon()
{
  return Building->Stats.Portrait;
}

float UUIInProgressBuildingCounter::GetCooldownTotalTime()
{
  return Building->Stats.Cooldown;
}

void UUIInProgressBuildingCounter::SetBuilding(ABuilding* building)
{
  Building = building;
}

bool UUIInProgressBuildingCounter::Click()
{
  if( !Peasant )
  {
    error( "UUIInProgressBuildingCounter didn't have Peasant set" );
    return 0;
  }

  Peasant->CancelBuilding( UUICmdActionIndex );

  return 1;
}

void UUIInProgressBuildingCounter::OnCooldown()
{
  UUIActionCommand::OnCooldown();
}

void UUIInProgressBuildingCounter::Step( float t )
{
  // The step function pulls from the building in progress.
  // It doesn't use the t param because the building will pause
  // if the peasant leaves it.
  cooldown.Time = Building->TimeBuilding;
}

