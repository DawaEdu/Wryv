#include "Wryv.h"

#include "Building.h"
#include "GlobalFunctions.h"
#include "UnitsData.h"
#include "WryvGameInstance.h"

#include "UIInProgressResearchCounter.h"
#include "UIResearchCommand.h"

UUIInProgressResearchCounter::UUIInProgressResearchCounter( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  
}

UTexture* UUIInProgressResearchCounter::GetIcon()
{
  return research->GetIcon();
}

float UUIInProgressResearchCounter::GetCooldownTotalTime()
{
  return research->GetCooldownTotalTime();
}

void UUIInProgressResearchCounter::Set( UUIResearchCommand* researchObject )
{
  research = researchObject;
}

bool UUIInProgressResearchCounter::Click()
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

void UUIInProgressResearchCounter::OnCooldown()
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


