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
    FVector2D size, FVector2D spacing ) :
    StackPanel( "Sidepanel", texPanelBkg, FLinearColor::White )
  {
    Pad = spacing;

    portraits = new FlowPanel( "FlowPanel", PortraitTexture, 1, 1, FVector2D( size.X, size.Y / 4 ) );
    portraits->Align = TopCenter;
    StackBottom( portraits );

    stats = new StatsPanel( "Stats", SolidWidget::SolidWhiteTexture, FLinearColor(0.15,0.15,0.15,0.2) );
    StackBottom( stats );
    
    actions = new Actions( "Actions", FVector2D(size.X/3,size.X/3) );
    StackBottom( actions );

    minimap = new Minimap( MinimapTexture, 4.f, FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    StackBottom( minimap );
    
    // Add the leftBorder in as last child, because it takes up full height,
    // and stackpanel will stack it in below the border
    SolidWidget *leftBorder = new SolidWidget( "panel leftborder",
      FVector2D( 4, size.Y ), FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
    leftBorder->Margin = - Pad + FVector2D( -4, 0 );
    Add( leftBorder );

    recomputeSizeToContainChildren();
  }
  virtual ~SidePanel(){}
  void Set( vector<AGameObject*> objects )
  {
    portraits->Set( objects );
    // Pass a NULL if collection empty
    AGameObject* go = 0;
    if( objects.size() )
      go = objects[0];
    stats->Set( go );
    actions->Set( go );
  }
};

