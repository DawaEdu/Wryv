#include "Wryv.h"
#include "SidePanel.h"

UTexture* SidePanel::RightPanelTexture = 0;

SidePanel::SidePanel( FVector2D size, FVector2D spacing ) :
  StackPanel( "SidePanel", RightPanelTexture, FLinearColor::White )
{
  Align = TopRight;
  Pad = spacing;

  Portraits = new PortraitsPanel( FVector2D( size.X, size.X ) );
  StackBottom( Portraits, HFull );

  Stats = new StatsPanel();
  StackBottom( Stats, HFull );
  
  actions = new ActionsPanel( "ActionsPanel", FVector2D( size.X/3, size.X/3 ) );
  StackBottom( actions, HFull );

  minimap = new Minimap( 4.f, FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
  StackBottom( minimap, HCenter );
    
  // Add the leftBorder in as last child, because it takes up full height,
  // and stackpanel will stack it in below the border
  SolidWidget* leftBorder = new SolidWidget( "panel leftborder",
    FVector2D( 4, size.Y ), FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
  leftBorder->Margin = - Pad + FVector2D( -4, 0 );
  Add( leftBorder );

  recomputeSizeToContainChildren();

  controls = new Controls();
  Add( controls );

  OnMouseDownLeft = [this](FVector2D mouse) -> EventCode {
    //LOG( "Sidepanel Absorbed click" );
    return Consumed;
  };
  OnMouseUpLeft = [this](FVector2D mouse) -> EventCode {
    //LOG( "Sidepanel Absorbed mouse up" );
    return Consumed;
  };
  OnMouseDragLeft = [this](FVector2D mouse) -> EventCode {
    //LOG( "Sidepanel Absorbed mouse drag" );
    return Consumed;
  };

}

void SidePanel::Set( vector<AGameObject*> objects )
{
  AGameObject* go = first( objects );
  Portraits->Set( objects );
  Stats->Set( go );
  actions->Set( go );
}
