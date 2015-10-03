#include "Wryv.h"

#include "Building.h"
#include "GlobalFunctions.h"
#include "Team.h"
#include "Research.h"

UResearch::UResearch( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Building = 0;
  NextLevel = 0;
}

void UResearch::Click()
{
  // Start the clock on there
  if( !cooldown.Done() )
  {
    // Spend money required to upgrade
    Building->team->Gold   -= GoldCost;
    Building->team->Lumber -= LumberCost;
    Building->team->Stone  -= StoneCost;
    cooldown.Reset();
  }
  
}

void UResearch::OnComplete()
{
  // Tell team that research is complete
  UAction::OnComplete();
  Building->team->CompletedResearches.push_back( this );
  
  // Change research icon in the UI to the next one in the string of UResearches
  // Get the element instance inside building's researches that has this object
  // and replace with the next one
  int i = Building->Researches.IndexOfByKey( GetClass() );

  if( i != -1 )
  {
    Building->CountersResearch[ i ] = NewObject<UResearch>( Building, NextLevel );
  }
  else
  {
    error( FS( "UResearch OnComplete OOB %s", *GetClass()->GetName() ) );
  }
}


