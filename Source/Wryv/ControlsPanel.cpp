#include "Wryv.h"
#include "ControlsPanel.h"
#include "WryvGameInstance.h"
#include "PlayerControl.h"

UTexture* ControlsPanel::PauseButtonTexture = 0;
UTexture* ControlsPanel::ResumeButtonTexture = 0;

ControlsPanel::ControlsPanel() : StackPanel( "ControlsPanel" )
{
  Pause = new ImageHS( "Pause", PauseButtonTexture );
  Align = TopRight;
  StackRight( Pause, Top );

  Pause->OnMouseDownLeft = [this]( FVector2D mouse ){
    // Pauses or unpauses the game
    Game->pc->SetPause( !Game->pc->IsPaused() );
    if( Game->pc->IsPaused() )
      Pause->Tex = ResumeButtonTexture;
    else  Pause->Tex = PauseButtonTexture;
    return Consumed;
  };
}

