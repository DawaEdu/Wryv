#include "Wryv.h"
#include "GameCanvas.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"
#include "PlayerControl.h"
#include "FlyCam.h"
#include "Building.h"

FCursorTexture GameCanvas::MouseCursorHand;
FCursorTexture GameCanvas::MouseCursorCrossHairs;

GameCanvas::GameCanvas( FVector2D size ) : Screen( "GameCanvas", size )
{
  selectBox = new MouseSelectBox( "mouse's select box",
    FBox2DU( FVector2D(100, 100), FVector2D(50, 50) ),
    8.f, FLinearColor::Green);
  Add( selectBox );
  selectBox->Hide();

  cursor = new ImageWidget( "mouse cursor", MouseCursorHand.Texture );
  Add( cursor );

  // Attach functionality
  OnMouseDownLeft = [this]( FVector2D mouse )
  {
    ABuilding *ghost = Game->flycam->ghost;
    if( ghost )
    {
      // Places the selected object without forming a new selection.
      if( ghost->CanBePlaced() )
      {
        info( FS( "Placing building %s", *ghost->Stats.Name ) );
        // Build the building. If ghost doesn't intersect any existing buildings then place it.
        if( ghost->team->CanAfford( ghost->Stats.Type ) )
        {
          ghost->team->Spend( ghost->Stats.Type );
          
          // Get the first Peasant object in selected
          if( Game->hud->Selected.size() )
          {
            if( APeasant *peasant = Cast<APeasant>( *Game->hud->Selected.begin() ) )
            {
              ghost->PlaceBuilding( peasant );
              Game->flycam->ghost = 0; // stop moving the ghost.
            }
            else
            {
              error( "A peasant wasn't selected to place the building." );
            }
          }
        }
      }
      else
      {
        // Ghost cannot be placed
        LOG( "Cannot place building here" );
      }
    }
    else
    {
      SelectStart( mouse );
    }
    return Consumed;
  };
  OnMouseUpLeft = [this]( FVector2D mouse ) {
    Game->hud->Select( Game->pc->Pick( selectBox->Box ) );
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
  
