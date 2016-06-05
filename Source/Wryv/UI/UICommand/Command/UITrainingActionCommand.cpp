#include "Wryv.h"

#include "GameObjects/Buildings/Building.h"
#include "GameObjects/GameObject.h"
#include "Util/GlobalFunctions.h"
#include "UE4/WryvGameInstance.h"

#include "UI/UICommand/Counters/UIInProgressUnitCounter.h"
#include "UI/UICommand/Command/UITrainingActionCommand.h"

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


