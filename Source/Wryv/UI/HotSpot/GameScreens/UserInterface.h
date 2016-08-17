#pragma once

#include "GameChrome.h"
#include "MapSelectionScreen.h"
#include "MissionObjectivesScreen.h"
#include "UI/HotSpot/Elements/Border.h"
#include "UI/HotSpot/Elements/Screen.h"
#include "UI/HotSpot/GameElements/StatusBar.h"
#include "TitleScreen.h"

// The root UI component
class UserInterface : public Screen
{
public:
  StatusBar*                statusBar;
  GameChrome*               gameChrome;
  Clock* clock;
  // In-game screens.
  TitleScreen*              titleScreen;
  MapSelectionScreen*       mapSelectionScreen;
  MissionObjectivesScreen*  missionObjectivesScreen;
  
  HotSpot*                  drag;
  bool                      dirty;

  UserInterface( FVector2D size );
  // Don't need to explicitly delete children of the UI object,
  // they get deleted in the base dtor.
  virtual ~UserInterface(){ /* no delete */ }
  void SetScreen( int mode );
  void SetSize( FVector2D size );
  virtual void Update( float t ) override;
};

