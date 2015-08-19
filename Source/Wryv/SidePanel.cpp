#include "Wryv.h"
#include "SidePanel.h"

UTexture* SidePanel::RightPanelTexture = 0;

SidePanel::SidePanel( FVector2D size, FVector2D spacing ) :
  StackPanel( "SidePanel", RightPanelTexture, FLinearColor::White )
{
  Align = TopRight;
  Pad = spacing;

  Portraits = new FlowPanel( "FlowPanel", 0, 1, 1, FVector2D( size.X, size.Y / 4 ) );
  Portraits->Align = TopCenter;
  StackBottom( Portraits );

  Stats = new StatsPanel( "Stats", SolidWidget::SolidWhiteTexture, FLinearColor(0.15, 0.15, 0.15, 0.2) );
  StackBottom( Stats );
    
  actions = new Actions( "Actions", FVector2D( size.X/3, size.X/3 ) );
  StackBottom( actions );

  minimap = new Minimap( 4.f, FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
  StackBottom( minimap );
    
  // Add the leftBorder in as last child, because it takes up full height,
  // and stackpanel will stack it in below the border
  SolidWidget* leftBorder = new SolidWidget( "panel leftborder",
    FVector2D( 4, size.Y ), FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
  leftBorder->Margin = - Pad + FVector2D( -4, 0 );
  Add( leftBorder );

  recomputeSizeToContainChildren();

  controls = new Controls();
  Add( controls );

}

void SidePanel::Set( set<AGameObject*> objects )
{
  AGameObject* go = first( objects );
  Portraits->Set( objects );
  Stats->Set( go );
  actions->Set( go );
}
