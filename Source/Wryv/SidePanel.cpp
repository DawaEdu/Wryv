#include "Wryv.h"
#include "SidePanel.h"

SidePanel::SidePanel( UTexture* texPanelBkg, UTexture* PortraitTexture, UTexture* MinimapTexture,
  FVector2D size, FVector2D spacing ) :
  StackPanel( "SidePanel", texPanelBkg, FLinearColor::White )
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

void SidePanel::Set( set<AGameObject*> objects )
{
  AGameObject* go = first( objects );
  portraits->Set( objects );
  stats->Set( go );
  actions->Set( go );
}
