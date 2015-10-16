#pragma once

#include "Screen.h"
#include "ResourcesWidget.h"
#include "SidePanel.h"
#include "CostWidget.h"
#include "ItemBelt.h"
#include "Buffs.h"
#include "BuildQueue.h"
#include "Controls.h"
#include "GameCanvas.h"
#include "Tooltip.h"

// The in-game chrome object, consisting of many subpanels etc
// +---------------------------------+
// | resourcesW        |  rightPanel |
// |                   |  Portrait   |
// |                   |  unitStats  |
// |                   |  abilities  |
// |                   | +---------+ |
// |  +-------------+  | | minimap | |
// |  | itemBelt/bq |  | +---------+ |
// +--buffs--------------------------+
class GameChrome : public Screen
{
public:
  ResourcesWidget*  resources;    // The resources widget in the top left
  SidePanel*        rightPanel;   // containing: portrait, unitStats, abilities, minimap
  ItemBelt*         itemBelt;     // things this unit is carrying, at the bottom of the screen
  BuildQueue*       buildQueue;   // queue of things we are building (in order)
  CostWidget*       costWidget;   // a flyover cost of the hovered item from 
  Tooltip*          tooltip;      // 
  GameCanvas*       gameCanvas;   // like acetate sheet user uses to select units etc.
  vector<AGameObject*> Selected;

  GameChrome( FString name, FVector2D size );
  void Select( vector<AGameObject*> objects );
};


