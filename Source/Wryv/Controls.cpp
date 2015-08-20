#include "Wryv.h"
#include "Controls.h"
#include "WryvGameInstance.h"
#include "PlayerControl.h"

UTexture* Controls::PauseButtonTexture = 0;
UTexture* Controls::ResumeButtonTexture = 0;

Controls::Controls() : StackPanel( "Controls" )
{
  Pause = new ImageWidget( "Pause", PauseButtonTexture );
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

