#pragma once

#include "Screen.h"
#include "ResourcesPanel.h"
#include "SidePanel.h"
#include "CostPanel.h"
#include "ItemBelt.h"
#include "BuffsPanel.h"
#include "BuildQueuePanel.h"
#include "ControlsPanel.h"
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
  ResourcesPanel*  resources;    // The resources widget in the top left
  SidePanel*        rightPanel;   // containing: portrait, unitStats, abilities, minimap
  ItemBelt*         itemBelt;     // things this unit is carrying, at the bottom of the screen
  BuildQueuePanel*       buildQueue;   // queue of things we are building (in order)
  CostPanel*       costWidget;   // a flyover cost of the hovered item from 
  Tooltip*          tooltip;      // 
  GameCanvas*       gameCanvas;   // like acetate sheet user uses to select units etc.
  vector<AGameObject*> Selected;

  GameChrome( FString name, FVector2D size );
  void Update( float t );
  void Select( vector<AGameObject*> objects );
};


