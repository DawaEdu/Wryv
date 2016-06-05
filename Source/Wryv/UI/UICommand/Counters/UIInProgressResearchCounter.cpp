#include "Wryv.h"

#include "GameObjects/Buildings/Building.h"
#include "Util/GlobalFunctions.h"
#include "Game/UnitsData.h"
#include "UE4/WryvGameInstance.h"

#include "UI/UICommand/Counters/UIInProgressResearchCounter.h"
#include "UI/UICommand/Command/UIResearchCommand.h"

UUIInProgressResearch::UUIInProgressResearch( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  
}

UTexture* UUIInProgressResearch::GetIcon()
{
  return research->GetIcon();
}

float UUIInProgressResearch::GetCooldownTotalTime()
{
  return research->GetCooldownTotalTime();
}

void UUIInProgressResearch::Set( UUIResearchCommand* researchObject )
{
  research = researchObject;
}

bool UUIInProgressResearch::Click()
{
  // stops the ressearch & refunds the money
  if( !cooldown.Done() )
  {
    info( FS( "Cancelled research %s", *Text ) );
    research->Building->CountersResearchInProgress.Remove( this );
    Team* team = research->Building->team;
    team->ResourceChange( research->Cost );
    cooldown.Reset();
  }
  return 1;  
}

void UUIInProgressResearch::OnCooldown()
{
  // Tell team that research is complete
  UUIActionCommand::OnCooldown();
  if( !research->Building ) {
    error( "Building not set" );
    return;
  }
  
  research->Building->team->CompletedResearches.push_back( research );
  
  // UUICmdActionIndex refers to the index inside the Building's research
  // classes queue.
  // Get the element instance inside building's researches that has this object
  // and replace with the NextLevel
  research->Building->ResearchClasses[ UUICmdActionIndex ] = research->NextLevel;
  research->Building->ResearchesAvailable[ UUICmdActionIndex ] = Construct<UUIResearchCommand>( research->NextLevel );

}


