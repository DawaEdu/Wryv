#pragma once

#include "UI/HotSpot/GamePanels/ActionsPanel.h"
#include "UI/HotSpot/Elements/Border.h"
#include "UI/HotSpot/GamePanels/ControlsPanel.h"
#include "UI/HotSpot/Panels/FlowPanel.h"
#include "UI/HotSpot/GameElements/Minimap.h"
#include "UI/HotSpot/GamePanels/PortraitsPanel.h"
#include "UI/HotSpot/Panels/StackPanel.h"
#include "UI/HotSpot/GamePanels/StatsPanel.h"

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

