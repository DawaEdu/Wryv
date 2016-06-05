#include "Wryv.h"

#include "UI/UICommand/Command/UIResearchCommand.h"

#include "GameObjects/Buildings/Building.h"
#include "Util/GlobalFunctions.h"
#include "Game/Team.h"

#include "UI/UICommand/Counters/UIInProgressResearchCounter.h"

UUIResearchCommand::UUIResearchCommand( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Text = "Unnamed research item";
  Building = 0;
  NextLevel = 0;
}

bool UUIResearchCommand::Click()
{
  Building->UseResearch( UUICmdActionIndex );
  return 1;
}

