#include "Wryv.h"
#include "GameChrome.h"

GameChrome::GameChrome( FString name, FVector2D size ) : Screen( name, size )
{
  resources = new ResourcesWidget( "Player's Resources", 16, 4 );
  Add( resources );

  rightPanel = new SidePanel( FVector2D( 280, size.Y ), FVector2D(8,8) );
  Add( rightPanel );

  itemBelt = new ItemBelt( SlotPalette::SlotPaletteTexture, 1, 4, FVector2D( 100, 100 ), FVector2D( 8, 8 ) );
  Add( itemBelt );

  buffs = new Buffs( "Buffs", 0 );
  Add( buffs );

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

void GameChrome::Select( vector<AGameObject*> objects )
{
  Selected = objects;
  AGameObject* go = first( Selected );
  rightPanel->Set( Selected );
  itemBelt->Set( Cast<AUnit>(go) );
  buildQueue->Set( go );
  buffs->Set( go );
}

