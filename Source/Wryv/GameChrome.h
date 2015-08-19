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
  Buffs*            buffs;        // list of buffs applied to this unit
  BuildQueue*       buildQueue;   // queue of things we are building (in order)
  CostWidget*       costWidget;   // a flyover cost of the hovered item from 
  ITextWidget*      tooltip;      // 
  GameCanvas*       gameCanvas;   // like acetate sheet user uses to select units etc.
  set<AGameObject*> Selected;
  static UTexture*  TooltipBackgroundTexture;

  GameChrome( FString name, FVector2D size );
  void Select( set<AGameObject*> objects );
};


