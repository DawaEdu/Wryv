#pragma once

#include "Screen.h"
#include "TitleScreen.h"
#include "GameChrome.h"
#include "MapSelectionScreen.h"
#include "MissionObjectivesScreen.h"
#include "StatusBar.h"
#include "Border.h"

// The root UI component
class UserInterface : public Screen
{
public:
  TitleScreen*              titleScreen;
  GameChrome*               gameChrome;
  MapSelectionScreen*       mapSelectionScreen;
  MissionObjectivesScreen*  missionObjectivesScreen;
  StatusBar*                statusBar;
  MouseSelectBox*           selectBox;
  ImageWidget*              mouseCursor;

  UserInterface( FVector2D size );
  // Don't need to explicitly delete children of the UI object,
  // they get deleted in the base dtor.
  virtual ~UserInterface(){ /* no delete */ }
  void SetScreen( int mode );
};

