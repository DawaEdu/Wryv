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

UTexture* UInProgressBuilding::GetIcon()
{
  return Building->Stats.Portrait;
}

float UInProgressBuilding::GetCooldownTotalTime()
{
  return Building->Stats.Cooldown;
}

void UInProgressBuilding::SetBuilding(ABuilding* building)
{
  Building = building;
}

bool UInProgressBuilding::Click()
{
  if( !Peasant )
  {
    error( "UInProgressBuilding didn't have Peasant set" );
    return 0;
  }

  Peasant->CancelBuilding( UActionIndex );

  return 1;
}

void UInProgressBuilding::OnCooldown()
{
  UAction::OnCooldown();
}

void UInProgressBuilding::Step( float t )
{
  // The step function pulls from the building in progress.
  // It doesn't use the t param because the building will pause
  // if the peasant leaves it.
  cooldown.Time = Building->TimeBuilding;
}