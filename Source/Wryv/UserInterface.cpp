#include "Wryv.h"
#include "UserInterface.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

UserInterface::UserInterface( FVector2D size ) :
  Screen( "UI-root", size )
{
  titleScreen = 0;
  gameChrome = new GameChrome( "GameChrome", size );
  Add( gameChrome );
  mapSelectionScreen = 0, missionObjectivesScreen = 0;
  // Initialize the status bar
  statusBar = new StatusBar( FLinearColor::Black );
  Add( statusBar );
  HotSpot::TooltipWidget = statusBar->Text;

}

void UserInterface::SetScreen( int mode )
{
  // hide elts not to be shown
  titleScreen->Hide();
  gameChrome->Hide();
  mapSelectionScreen->Hide();
  missionObjectivesScreen->Hide();
    
  // Then go ahead and show elts depending on mode we're in
  if( mode == Title )  titleScreen->Show();
  else if( mode == MapSelect )  mapSelectionScreen->Show();
  else if( mode == Running )  gameChrome->Show();

  statusBar->Show(); // Show the status bar at all times
}

void UserInterface::SetSize( FVector2D size )
{
  Size = size;
  gameChrome->Size = size;
}




