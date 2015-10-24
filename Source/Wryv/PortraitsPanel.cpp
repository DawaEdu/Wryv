#include "Wryv.h"

#include "TheHUD.h"
#include "PortraitsPanel.h"
#include "WryvGameInstance.h"

int PortraitsPanel::MaxPortraits = 25;

PortraitsPanel::PortraitsPanel( FVector2D size ) : FlowPanel( "PortraitsPanel", 0, 1, 1, size )
{
  for( int i = 0; i < MaxPortraits; i++ )
  {
    Portrait* p = new Portrait( FS("Portrait %d", i), NoTextureTexture, 3.f, FLinearColor(.2f,.2f,.2f,1.f) );
    Add( p );
  }

  HideChildren();
}

Portrait* PortraitsPanel::GetPortrait( int i )
{
  if( i < 0 || i >= children.size() )
  {
    error( FS( "PortraitsPanel::GetPortrait( %d ) OOB", i ) );
    return 0;
  }

  return (Portrait*)children[i];
}

void PortraitsPanel::Set( vector<AGameObject*> objects )
{
  // Null all object refs.
  for( int i = 0; i < children.size(); i++ )
    GetPortrait(i)->Set( 0 );
  
  HideChildren();
  Cols = Rows = ceilf( sqrtf( objects.size() ) ) ;
  if( !Cols ) return; //empty

  Reflow();

  for( int i = 0; i < objects.size(); i++ )
  {
    Portrait *p = GetPortrait( i );
    if( !p )
    {
      error( FS( "Cannot select more than %d units due to PP settings", children.size() ) );
      return;
    }
    
    p->Set( objects[i] );
  }
}


