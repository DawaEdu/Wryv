#include "Wryv.h"
#include "UserInterface.h"

UserInterface::UserInterface( FVector2D size ) : Screen( "UI-root" )
{
  Size = size;
  titleScreen = 0;
  gameChrome = new GameChrome( "GameChrome" );
  Add( gameChrome );
  mapSelectionScreen = 0, missionObjectivesScreen = 0;
  // Initialize the status bar
  statusBar = new StatusBar( FLinearColor::Black );
  Add( statusBar );
  HotSpot::TooltipWidget = statusBar->Text; // Setup the default tooltip location
  // connect the mouse drag functions 
  selectBox = new MouseSelectBox( "MouseSelectBox border",
    FBox2DU( FVector2D(100,100), FVector2D(50,50) ),
    8.f, FLinearColor::Green);
  Add( selectBox );
  selectBox->Hide();
  mouseCursor = 0;
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
  mouseCursor->Show();
}


