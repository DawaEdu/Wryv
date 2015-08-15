#pragma once

#include "StackPanel.h"
#include "FlowPanel.h"
#include "StatsPanel.h"
#include "ActionsPanel.h"
#include "Border.h"
#include "StatsPanel.h"

class Minimap : public ImageWidget
{
  Border* borders;
public:
  Minimap( UTexture* icon, float borderSize, FLinearColor borderColor ) : 
    ImageWidget( "minimap", icon )
  {
    FBox2DU box( FVector2D(0,0), Size );
    borders = new Border( "Minimap border", box, borderSize, borderColor );
    Add( borders );
  }
  virtual ~Minimap(){}
};

// The right-side panel
class SidePanel : public StackPanel
{
public:
  // +-----------+
  // | portrait  |
  // +-----------+
  // | Stats     |
  // +-----------+
  // | actions   |
  // +-----------+
  // | minimap   |
  // +-----------+
  FlowPanel* portraits;    // pictoral representation of selected unit
  StatsPanel* stats;  // The stats of the last selected unit
  Actions* actions; // contains both the abilities & buildings pages
  Minimap* minimap;       // the minimap widget for displaying the world map

  // For the group of selected units.
  SidePanel( UTexture* texPanelBkg, UTexture* PortraitTexture, UTexture* MinimapTexture,
    FVector2D size, FVector2D spacing );
  virtual ~SidePanel() { }
  void Set( set<AGameObject*> objects );
};

