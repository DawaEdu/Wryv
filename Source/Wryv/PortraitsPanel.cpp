#include "Wryv.h"
#include "PortraitsPanel.h"
#include "WryvGameInstance.h"
#include "FlyCam.h"

PortraitsPanel::PortraitsPanel( FVector2D size ) :
  FlowPanel( "PortraitsPanel", 0, 1, 1, size )
{
  
}

void PortraitsPanel::Set( set<AGameObject*> objects )
{
  // Change rows/cols.
  Cols = Rows = ceilf( sqrtf( objects.size() ) ) ;
  Clear();
  if( !Cols ) return; //empty

  for( AGameObject* go : objects ) {
    ImageWidget* im = new ImageWidget( go->Stats.Name, go->Stats.Portrait );
    Add( im ) ;
    im->OnMouseDownLeft = [go]( FVector2D mouse ){
      // re-select this object
      set<AGameObject*> s;
      s.insert( go );
      Game->flycam->Select( s );
      return Consumed;
    };
  }
  if( Rows && Cols ) reflow();
}

