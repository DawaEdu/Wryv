#include "Wryv.h"

#include "Building.h"
#include "BuildAction.h"
#include "BuildQueue.h"
#include "Clock.h"
#include "GlobalFunctions.h"
#include "Peasant.h"
#include "SlotPalette.h"
#include "Unit.h"

#include "InProgressBuilding.h"
#include "InProgressUnit.h"
#include "InProgressResearch.h"
#include "Research.h"
#include "TrainingAction.h"

BuildQueue::BuildQueue( FString name, FVector2D entrySize, int maxTrains ) : 
  StackPanel( name, StackPanel::StackPanelTexture ), EntrySize( entrySize )
{
  Pad = FVector2D(4,4);
  Align = BottomCenter;
  Size = FVector2D(0,0);

  // Initialize with 10 clocks (max trains)
  for( int i = 0; i < maxTrains; i++ )
  {
    Clock *clock = new Clock( FS( "Units in progress %d", i ), EntrySize, NoTextureTexture, 
      FLinearColor::White, Alignment::CenterCenter );
    StackRight( clock, VCenter );
  }
}

void BuildQueue::Set( vector<AGameObject*> objects )
{
  // When you call SET, you clear all the old buildcounters.
  // Clocks inside the buildcounter are not cached.
  HideChildren(); // Assume hidden

  for( int i = 0; i < objects.size(); i++ )
  {
    AGameObject* go = objects[i];
    if( ABuilding* building = Cast<ABuilding>( go ) )
    {
      // How many clocks are needed in the build queue?
      int numClocks = building->CountersUnitsInProgress.Num() + building->CountersResearchInProgress.Num();
      if( !numClocks )  return; // No clocks to show.
      
      // Populate with the Units in progress first if any.
      for( int i = 0; i < building->CountersUnitsInProgress.Num(); i++ )
      {
        UInProgressUnit* inProgress = building->CountersUnitsInProgress[i];
        Clock* clock = (Clock*)children[i];
        inProgress->PopulateClock( clock, i );
      }

      // Populate with any ResearchInProgress.
      int j = building->CountersUnitsInProgress.Num();
      for( int i = 0; i < building->CountersResearchInProgress.Num(); i++ )
      {
        UInProgressResearch* research = building->CountersResearchInProgress[i];
        Clock* clock = (Clock*)children[i + j];
        research->PopulateClock( clock, i + j );
      }
    }
    else if( APeasant* peasant = Cast<APeasant>( go ) ) // Throw in also any peasant's buildqueue
    {
      for( int i = 0; i < peasant->CountersBuildingsQueue.Num(); i++ )
      {
        UInProgressBuilding* buildingInProgress = peasant->CountersBuildingsQueue[i];
        Clock* clock = (Clock*)children[i];
        buildingInProgress->PopulateClock( clock, i );
      }
    }
  }
}

