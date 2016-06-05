#include "Wryv.h"

#include "GameObjects/Buildings/Building.h"
#include "GameObjects/Units/Peasant.h"
#include "Util/GlobalFunctions.h"
#include "UI/UICommand/Counters/UIInProgressBuildingCounter.h"

UUIInProgressBuilding::UUIInProgressBuilding( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Building = 0;
  Peasant = 0;
}

UTexture* UUIInProgressBuilding::GetIcon()
{
  return Building->Stats.Portrait;
}

float UUIInProgressBuilding::GetCooldownTotalTime()
{
  return Building->Stats.Cooldown;
}

void UUIInProgressBuilding::SetBuilding(ABuilding* building)
{
  Building = building;
}

bool UUIInProgressBuilding::Click()
{
  if( !Peasant )
  {
    error( "UUIInProgressBuilding didn't have Peasant set" );
    return 0;
  }

  Peasant->CancelBuilding( UUICmdActionIndex );

  return 1;
}

void UUIInProgressBuilding::OnCooldown()
{
  UUIActionCommand::OnCooldown();
}

void UUIInProgressBuilding::Step( float t )
{
  // The step function pulls from the building in progress.
  // It doesn't use the t param because the building will pause
  // if the peasant leaves it.
  cooldown.Time = Building->TimeBuilding;
}

