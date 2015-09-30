#include "Wryv.h"
#include "GameCanvas.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"
#include "PlayerControl.h"
#include "FlyCam.h"
#include "Building.h"
#include "Peasant.h"

FCursorTexture GameCanvas::MouseCursorHand;
FCursorTexture GameCanvas::MouseCursorCrossHairs;

GameCanvas::GameCanvas( FVector2D size ) : Screen( "GameCanvas", size )
{
  selectBox = new MouseSelectBox( "mouse's select box",
    FBox2DU( FVector2D(100, 100), FVector2D(50, 50) ),
    4.f, FLinearColor::Green);
  Add( selectBox );
  selectBox->Hide();

  cursor = new ImageWidget( "mouse cursor", MouseCursorHand.Texture );
  Add( cursor );

  // Attach functionality
  OnMouseDownLeft = [this]( FVector2D mouse )
  {
    // Presence of the ghost indicates a building is queued to be placed by the UI.
    if( !Game->flycam->ghost )
    {
      // Box-shaped selection here
      SelectStart( mouse );
    }
    else
    {
      // Places the selected object without forming a new selection.
      ABuilding *ghost = Game->flycam->ghost;

      // the src peasant must be present
      APeasant *peasant = 0;
      if( Game->hud->Selected.size() )
      {
        peasant = Cast<APeasant>( Game->hud->Selected[0] );
      }
      if( !peasant )
      {
        error( FS( "A peasant wasn't selected to place the building.", *peasant->GetName() ) );
        return NotConsumed;
      }

      // If the user has UI-placed the building in an acceptable spot,
      // then we should place the building here.
      if( ghost->CanBePlaced() )
      {
        // Build a building @ location. The peasant will pick up this command next frame.
        Command cmd( Command::CommandType::CreateBuilding,
          peasant->ID, peasant->LastBuildingID, ghost->Pos );
        Game->EnqueueCommand( cmd );
        Game->flycam->ClearGhost();
      }
      else
      {
        // Ghost cannot be placed
        LOG( "Cannot place building here" );
      }
    }
    
    return Consumed;
  };
  OnMouseUpLeft = [this]( FVector2D mouse ) {
    // Box shaped selection of units.
    // Filtration is done after selection,
    Game->hud->Select( Game->pc->FrustumPick( selectBox->Box ) );
    SelectEnd();
    return Consumed;
  };
  OnHover = [this]( FVector2D mouse ) {
    // Regular hover event
    Set( mouse );
    return NotConsumed;
  };
  OnMouseDragLeft = [this]( FVector2D mouse ) {
    DragBox( mouse );
    return Consumed;
  };
}

void GameCanvas::Set( FVector2D mouse )
{
  // Sets with mouse cursor position
  cursor->Margin = mouse;
}

void GameCanvas::SelectStart( FVector2D mouse )
{
  selectBox->SetStart( mouse );
  selectBox->Show();
  cursor->Hide();
}

void GameCanvas::DragBox( FVector2D mouse )
{
  selectBox->SetEnd( mouse );
}

void GameCanvas::SelectEnd()
{
  selectBox->Hide();
  cursor->Show();
}
  
