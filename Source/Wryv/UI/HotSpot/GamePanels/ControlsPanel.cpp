#include "Wryv.h"
#include "UI/HotSpot/GamePanels/ControlsPanel.h"
#include "UE4/WryvGameInstance.h"
#include "UE4/PlayerControl.h"

UTexture* ControlsPanel::PauseButtonTexture = 0;
UTexture* ControlsPanel::ResumeButtonTexture = 0;

ControlsPanel::ControlsPanel() : StackPanel( "ControlsPanel" )
{
  Pause = new Image( "Pause", PauseButtonTexture );
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

