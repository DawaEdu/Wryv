#include "Wryv.h"
#include "BuildQueue.h"
#include "Clock.h"

FLinearColor BuildQueue::ClockColor( 0.2, 0.2, 0.2, .1 );

BuildQueue::BuildQueue( FString name, FVector2D entrySize ) : 
  StackPanel( name, StackPanel::StackPanelTexture ), EntrySize( entrySize )
{
  Pad = FVector2D( 4, 4 );
  Align = BottomCenter;
  Selected = 0;
  needsRefresh = 0;
  Size = FVector2D(0,0);
}

void BuildQueue::Set( AGameObject* go )
{
  needsRefresh = 0;
  Selected = go;
  // When you call SET, you clear all the old buildcounters.
  // Clocks inside the buildcounter are not cached.
  Clear();
  if( !go )  return;

  // Things that are spawning each have a clock.
  for( int i = 0; i < go->BuildQueueCounters.size(); i++ )
  {
    CooldownCounter cd = go->BuildQueueCounters[i];
    Clock* clock = new Clock( Game->unitsData[ cd.Type ].Name + FString( "'s cooldown" ),
      EntrySize, cd, ClockColor );
    StackRight( clock );
    
    // Register a function to remove the widget from this queue when clicked.
    clock->OnMouseDownLeft = [this,go,clock,i](FVector2D mouse)
    {
      LOG( "Cancelled item %s [%d] belonging to %s",
        *GetTypesName( clock->counter.Type ), i, *go->Stats.Name );
      removeIndex( go->BuildQueueCounters, i );
      needsRefresh = 1; // reset the buildQueue's contents.
      return Consumed;
    };
  }
}

void BuildQueue::Refresh()
{
  needsRefresh=1;
}

void BuildQueue::Move( float t )
{
  if( needsRefresh ) {
    Set( Selected );
  }
}
