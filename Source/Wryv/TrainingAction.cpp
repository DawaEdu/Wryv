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
  return 0.f; // There is no cooldown for TrainingAction buttons.
}

bool UTrainingAction::Click()
{
  Building->UseTrain( UActionIndex );
  return 1;
}


