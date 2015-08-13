#pragma once

#include "Screen.h"
#include "TitleScreen.h"
#include "GameChrome.h"
#include "MapSelectionScreen.h"
#include "MissionObjectivesScreen.h"
#include "StatusBar.h"
#include "Border.h" // MouseSelectBox

// The root UI component
class UserInterface : public Screen
{
public:
  TitleScreen* titleScreen; // the title screen.
  GameChrome* gameChrome; // in-game chrome.
  MapSelectionScreen* mapSelectionScreen;
  MissionObjectivesScreen* missionObjectivesScreen;
  StatusBar* statusBar; // the bottom status bar
  MouseSelectBox* selectBox;
  ImageWidget* mouseCursor;

  UserInterface( FVector2D size ) : Screen( "UI-root" )
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

  // don't need to expli  citly delete children of the UI object,
  // they get deleted in the base dtor.
  virtual ~UserInterface(){ /* no delete */ }

  void SetScreen( int mode )
  {
    // This hides all direct children
    //HideChildren(); // hide all widgets, then show just the ones we want
    // The above line was dangerous because if you're adding more elts
    // because will have to explicitly show each below

    // explicitly hide elts not to be shown
    titleScreen->Hide(); // the title screen.
    gameChrome->Hide(); // in-game chrome.
    mapSelectionScreen->Hide();
    missionObjectivesScreen->Hide();
    
    // Then go ahead and show elts depending on mode we're in
    if( mode == Title )  titleScreen->Show();
    else if( mode == MapSelect )  mapSelectionScreen->Show();
    else if( mode == Running )  gameChrome->Show();

    statusBar->Show(); // Show the status bar at all times
    mouseCursor->Show();
  }

};

