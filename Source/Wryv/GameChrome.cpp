#include "Wryv.h"
#include "GameChrome.h"

GameChrome::GameChrome( FString name ) : Screen( name )
{
  resources = new ResourcesWidget( "Main ResourcesWidget", 16, 4 );
  Add( resources );

  rightPanel = 0, itemBelt = 0, buffs = 0, buildQueue = 0;
  costWidget = 0, tooltip = 0, controls = 0;
}

void GameChrome::Select( set<AGameObject*> objects )
{
  AGameObject* go = first( objects );
  rightPanel->Set( objects );
  itemBelt->Set( Cast<AUnit>(go) );
  buildQueue->Set( go );
  buffs->Set( go );
}
