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
  return (Portrait*)GetChild(i);
}

void PortraitsPanel::Set( vector<AGameObject*> objects )
{
  // Null all object refs.
  for( int i = 0; i < GetNumChildren(); i++ )
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
      error( FS( "Cannot select more than %d units due to PP settings", GetNumChildren() ) );
      return;
    }
    
    p->Set( objects[i] );
  }
}


