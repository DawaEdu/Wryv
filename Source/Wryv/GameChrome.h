#pragma once

#include "Screen.h"
#include "ResourcesWidget.h"
#include "SidePanel.h"
#include "CostWidget.h"
#include "ItemBelt.h"
#include "Buffs.h"
#include "BuildQueue.h"
#include "Controls.h"

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
  ResourcesWidget* resources; // The resources widget in the top left
  SidePanel* rightPanel;      // containing: portrait, unitStats, abilities, minimap
  CostWidget* costWidget;     // a flyover cost of the hovered item from 
  ITextWidget* tooltip;

  ItemBelt* itemBelt;     // things this unit is carrying, at the bottom of the screen
  Buffs* buffs;           // list of buffs applied to this unit
  BuildQueue* buildQueue; // queue of things we are building (in order)
  Controls* controls;     // 
  
  GameChrome( FString name ) : Screen( name )
  {
    resources = new ResourcesWidget( "Main ResourcesWidget", 16, 4 );
    Add( resources );

    rightPanel = 0, costWidget = 0, tooltip = 0, 
    itemBelt = 0, buffs = 0, buildQueue = 0, controls = 0;
  }

  void Select( vector<AGameObject*> objects );
};


