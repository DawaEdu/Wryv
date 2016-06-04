#include "Wryv.h"

#include "UIResearchCommand.h"

#include "Building.h"
#include "GlobalFunctions.h"
#include "Team.h"

#include "UIInProgressResearchCounter.h"

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

