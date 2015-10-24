#include "Wryv.h"

#include "GameChrome.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

GameChrome::GameChrome( FString name, FVector2D size ) : Screen( name, size )
{
  resources = new ResourcesWidget( "Player's Resources", 16, 4 );
  Add( resources );

  rightPanel = new SidePanel( FVector2D( 280, size.Y ), FVector2D(8,8) );
  Add( rightPanel );

  itemBelt = new ItemBelt( SlotPalette::SlotPaletteTexture, 1, 8, FVector2D( 100, 100 ), FVector2D( 8, 8 ) );
  Add( itemBelt );

  buildQueue = new BuildQueue( "Building Queue", FVector2D( 128, 128 ) );
  Add( buildQueue );
  
  costWidget = new CostWidget();
  Add( costWidget );
  costWidget->Align = CenterCenter;
  costWidget->Hide();

  tooltip = new Tooltip();
  Add( tooltip );
  tooltip->Pad = FVector2D( 8, 8 );
  tooltip->OnMouseDownLeft = [this](FVector2D mouse) -> EventCode {
    LOG( "Hiding the tooltip" );
    tooltip->Hide();
    return Consumed;
  };
  tooltip->Hide();

  // Canvas at bottom of thing so selections to sidepanel override & chance to consume
  // comes to them first.
  gameCanvas = new GameCanvas( size );
  Add( gameCanvas );
}

void GameChrome::Update( float t )
{
  // Update # resources available
  resources->SetValues( Game->gm->playersTeam->Gold, 
    Game->gm->playersTeam->Lumber, Game->gm->playersTeam->Stone );
}

void GameChrome::Select( vector<AGameObject*> objects )
{
  Selected = objects;
  rightPanel->Set( objects ); // portraits of all

  itemBelt->Set( objects );
  buildQueue->Set( objects );
}



