#include "Wryv.h"

#include "Building.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "WryvGameInstance.h"

#include "InProgressUnit.h"
#include "TrainingAction.h"

UTrainingAction::UTrainingAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

UTexture* UTrainingAction::GetIcon()
{
  return Game->GetData( UnitType ).Portrait;
}

float UTrainingAction::GetCooldownTotalTime()
{
  return Game->GetData( UnitType ).Cooldown;
}

bool UTrainingAction::Click()
{
  Building->UseTrain( UActionIndex );
  return 1;
}


