#include "Wryv.h"
#include "GameCanvas.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

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
    SelectStart( mouse );
    return Consumed;
  };
  OnMouseUpLeft = [this]( FVector2D mouse ){ 
    // Cannot select here, must in render call.
    Game->hud->WillSelectNextFrame = 1;
    if( Game->pc->IsAnyKeyDown( { EKeys::LeftShift, EKeys::RightShift } ) )
      Game->hud->selectionParity = Adds;
    else if( Game->pc->IsAnyKeyDown( { EKeys::LeftControl, EKeys::RightControl } ) )
      Game->hud->selectionParity = Subtracts;
    else
      Game->hud->selectionParity = NewSelection;
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
  
