#include "Wryv.h"

#include "Building.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "UnitsData.h"
#include "WryvGameInstance.h"

#include "InProgressResearch.h"
#include "Research.h"

UInProgressResearch::UInProgressResearch( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  
}

UTexture* UInProgressResearch::GetIcon()
{
  return research->GetIcon();
}

float UInProgressResearch::GetCooldownTotalTime()
{
  return research->GetCooldownTotalTime();
}

void UInProgressResearch::Set( UResearch* researchObject )
{
  research = researchObject;
}

bool UInProgressResearch::Click()
{
  // stops the ressearch & refunds the money
  if( !cooldown.Done() )
  {
    info( FS( "Cancelled research %s", *Text ) );
    research->Building->CountersResearchInProgress.Remove( this );
    Team *team = research->Building->team;
    team->Gold   += research->GoldCost;
    team->Lumber += research->LumberCost;
    team->Stone  += research->StoneCost;
    cooldown.Reset();
  }
  return 1;  
}

void UInProgressResearch::OnCooldown()
{
  // Tell team that research is complete
  UAction::OnCooldown();
  if( !research->Building ) {
    error( "Building not set" );
    return;
  }
  
  research->Building->team->CompletedResearches.push_back( research );
  
  // UActionIndex refers to the index inside the Building's research
  // classes queue.
  // Get the element instance inside building's researches that has this object
  // and replace with the NextLevel
  research->Building->ResearchClasses[ UActionIndex ] = research->NextLevel;
  research->Building->ResearchesAvailable[ UActionIndex ] = Construct<UResearch>( research->NextLevel );

}


