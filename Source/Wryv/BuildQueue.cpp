#include "Wryv.h"

#include "Building.h"
#include "BuildAction.h"
#include "BuildQueue.h"
#include "Clock.h"
#include "GlobalFunctions.h"
#include "Peasant.h"
#include "Unit.h"

#include "InProgressBuilding.h"
#include "Research.h"
#include "TrainingAction.h"

FLinearColor BuildQueue::ClockColor( 0,0,0,0.15 );

BuildQueue::BuildQueue( FString name, FVector2D entrySize ) : 
  StackPanel( name, StackPanel::StackPanelTexture ), EntrySize( entrySize )
{
  Pad = FVector2D( 4, 4 );
  Align = BottomCenter;
  Selected = 0;
  needsRefresh = 0;
  Size = FVector2D(0,0);
}

void BuildQueue::Clear()
{
  StackPanel::Clear();
  clocks.clear(); // clear extra references
}

void BuildQueue::Set( AGameObject* go )
{
  needsRefresh = 0;
  Selected = go;
  // When you call SET, you clear all the old buildcounters.
  // Clocks inside the buildcounter are not cached.
  Clear();
  Hide(); // Assume hidden

  if( ABuilding* building = Cast<ABuilding>( go ) )
  {
    if( building->CountersTraining.size() )
      Show(); // There is an object to show
    // Things that are spawning each have a clock.

    for( int i = 0; i < building->CountersTraining.size(); i++ )
    {
      UTrainingAction* trainingAction = building->CountersTraining[i];
      Clock* clock = new Clock( FS( "Cooldown %d", i ),
        EntrySize, trainingAction->Icon, ClockColor );
      StackRight( clock, VCenter );
      clocks.push_back( clock );
    
      // Register a function to remove the widget from this queue when clicked.
      clock->OnMouseDownLeft = [this,building,clock,i](FVector2D mouse) -> EventCode
      {
        LOG( "Cancelled item [%d] belonging to %s", i, *building->Stats.Name );
        removeIndex( building->CountersTraining, i );
        needsRefresh = 1; // reset the buildQueue's contents.
        return Consumed;
      };
    }

    if( building->CountersResearch.size() )
      Show(); // There is an object to show
    for( int i = 0; i < building->CountersResearch.size(); i++ )
    {
      UResearch* research = building->CountersResearch[i];
      Clock* clock = new Clock( FS( "Cooldown %d", i ),
        EntrySize, research->Icon, ClockColor );
      StackRight( clock, VCenter );
      clocks.push_back( clock );
      
      clock->OnMouseDownLeft = [this,building,clock,i](FVector2D mouse) -> EventCode
      {
        LOG( "Cancelled item [%d] belonging to %s", i, *building->Stats.Name );
        removeIndex( building->CountersResearch, i );
        needsRefresh = 1;
        return Consumed;
      };
    }
  }
  else if( APeasant* peasant = Cast<APeasant>( go ) ) // Throw in also any peasant's buildqueue
  {
    if( peasant->CountersBuildingsQueue.size() )
      Show(); // There is an object to show

    for( int i = 0; i < peasant->CountersBuildingsQueue.size(); i++ )
    {
      UInProgressBuilding* buildingInProgress = peasant->CountersBuildingsQueue[i];
      Clock* clock = new Clock( FS( "Cooldown %d", i ),
        EntrySize, buildingInProgress->Icon, ClockColor );
      StackRight( clock, VCenter );
      clocks.push_back( clock );
    
      // Register a function to remove the widget from this queue when clicked.
      clock->OnMouseDownLeft = [this,peasant,buildingInProgress,clock,i](FVector2D mouse) -> EventCode
      {
        info( FS( "Cancelled item [%d] belonging to %s",
          i, *buildingInProgress->Building->Stats.Name ) );
        removeIndex( peasant->CountersBuildingsQueue, i );
        needsRefresh = 1; // reset the buildQueue's contents.
        return Consumed;
      };
    }
  }
}

void BuildQueue::Refresh()
{
  needsRefresh = 1;
}

void BuildQueue::Move( float t )
{
  if( needsRefresh )
  {
    Set( Selected );
  }

  // Update clocks inside the selected building.
  if( ABuilding* building = Cast<ABuilding>( Selected ) )
  {
    // refresh the counts
    for( int i = 0; i < building->CountersTraining.size(); i++ )
    {
      if( i >= clocks.size() )
      {
        error( FS( "Index %d OOB the clocks", i ) );
        Set( Selected ); // reset the clocks
        break;
      }
      else
      {
        clocks[i]->Set( building->CountersTraining[i]->cooldown.Fraction() );
      }
    }
  }
}
