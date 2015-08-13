#pragma once

#include "StackPanel.h"
#include "GameObject.h"
#include "WryvGameInstance.h"

// This is the object that describes the units we are building
class BuildQueue : public StackPanel
{
public:
  FVector2D EntrySize;
  // A BuildQueue contains a list of things that are being built
  BuildQueue( FString name, UTexture* bkgTex, FVector2D entrySize ) : 
    StackPanel( name, bkgTex ), EntrySize( entrySize )
  {
  }

  void Set( AGameObject* go )
  {
    // Things that are spawning.
    for( int i = 0; i < go->BuildQueueCounters.size(); i++ )
    {
      Types type = go->BuildQueueCounters[i].Type;
      Clock* clock = new Clock( 
        Game->unitsData[ type ].Name + FString( "'s cooldown" ),
        EntrySize, CooldownCounter( type ),
        FLinearColor( 0.15, 0.15, 0.15, 0.15 ) );
      StackRight( clock );

      // Register a function to remove the widget from this queue when clicked.
      clock->OnMouseDownLeft = [this,go,clock,i](FVector2D mouse)
      {
        // Remove this entry from the buildQueue then refresh the spawn queue
        // This works perfectly fine, the index to remove is actually
        // correct since a call to Set() follows removal from the queue.
        removeIndex( go->BuildQueueCounters, i );
        Set( go ); // reset the buildQueue's contents.
        return Consumed;
      };
    }
  }


};
