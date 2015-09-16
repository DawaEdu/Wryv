#pragma once

#include "StackPanel.h"
#include "FlowPanel.h"
#include "StatsPanel.h"
#include "ActionsPanel.h"
#include "Border.h"
#include "StatsPanel.h"
#include "Minimap.h"
#include "Controls.h"
#include "PortraitsPanel.h"

// The right-side panel
class SidePanel : public StackPanel
{
public:
  // +---+-----------+
  // | > | Portrait  |
  // +---+-----------+
  //     | Stats     |
  //     +-----------+
  //     | actions   |
  //     +-----------+
  //     | minimap   |
  //     +-----------+
  PortraitsPanel* Portraits;   // pictoral representation of selected unit
  StatsPanel* Stats;      // The stats of the last selected unit
  ActionsPanel* actions;       // contains both the abilities & buildings pages
  Minimap* minimap;       // the minimap widget for displaying the world map
  Controls* controls;     // controls for pause/unpause/menu
  static UTexture* RightPanelTexture;

  // For the group of selected units.
  SidePanel( FVector2D size, FVector2D spacing );
  virtual ~SidePanel() { }
  void Set( vector<AGameObject*> objects );
};

