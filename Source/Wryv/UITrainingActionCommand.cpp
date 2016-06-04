#include "Wryv.h"

#include "Building.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "WryvGameInstance.h"

#include "UIInProgressUnitCounter.h"
#include "UITrainingActionCommand.h"

UUITrainingActionCommand::UUITrainingActionCommand( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

UTexture* UUITrainingActionCommand::GetIcon()
{
  return Game->GetData( UnitType ).Portrait;
}

float UUITrainingActionCommand::GetCooldownTotalTime()
{
  return 0.f; // There is no cooldown for TrainingAction buttons.
}

bool UUITrainingActionCommand::Click()
{
  Building->UseTrain( UUICmdActionIndex );
  return 1;
}


