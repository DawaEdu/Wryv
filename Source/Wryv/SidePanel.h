#pragma once

#include "ActionsPanel.h"
#include "Border.h"
#include "ControlsPanel.h"
#include "FlowPanel.h"
#include "Minimap.h"
#include "PortraitsPanel.h"
#include "StackPanel.h"
#include "StatsPanel.h"

// The right-side panel
class SidePanel : public StackPanel
{
public:
  // +---+-----------+
  // | > | Portrait  |
  // +---+-----------+
  //     | Stats     |
  //     +-----------+
  //     | Actions   |
  //     +-----------+
  //     | minimap   |
  //     +-----------+
  PortraitsPanel* Portraits;  // pictoral representation of selected unit
  StatsPanel* Stats;          // The stats of the last selected unit
  ActionsPanel* Actions;      // contains both the abilities & buildings pages
  Minimap* minimap;           // the minimap widget for displaying the world map
  Solid* leftBorder;
  ControlsPanel* controlsPanel;         // controlsPanel for pause/unpause/menu
  static UTexture* RightPanelTexture;

  // For the group of selected units.
  SidePanel( FVector2D size, FVector2D spacing );
  virtual ~SidePanel() { }
  void Set( vector<AGameObject*> objects );
  void Restack();
  virtual void render( FVector2D offset );
};

