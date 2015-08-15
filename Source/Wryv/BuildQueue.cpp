#include "Wryv.h"
#include "BuildQueue.h"
#include "Clock.h"

BuildQueue::BuildQueue( FString name, FVector2D entrySize ) : 
  StackPanel( name ), EntrySize( entrySize )
{
  Pad = FVector2D( 4, 4 );
  Align = BottomCenter;
}

void BuildQueue::Set( AGameObject* go )
{
  // When you call SET, you clear all the old buildcounters.
  // Clocks inside the buildcounter are not cached.
  Clear();
  if( !go )  return;

  // Things that are spawning each have a clock.
  for( int i = 0; i < go->BuildQueueCounters.size(); i++ )
  {
    CooldownCounter cd = go->BuildQueueCounters[i];
    Clock* clock = new Clock( Game->unitsData[ cd.Type ].Name + FString( "'s cooldown" ),
      EntrySize, cd, FLinearColor( 0.15, 0.15, 0.15, 0.15 ) );
    StackRight( clock );

    // Register a function to remove the widget from this queue when clicked.
    clock->OnMouseDownLeft = [this,go,clock,i](FVector2D mouse)
    {
      LOG( "Cancelled item %s [%d] belonging to %s",
        *GetTypesName( clock->counter.Type ), i, *go->Stats.Name );
      removeIndex( go->BuildQueueCounters, i );
      Set( go ); // reset the buildQueue's contents.
      return Consumed;
    };
  }
}


