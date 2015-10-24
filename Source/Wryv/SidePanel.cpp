#include "Wryv.h"

#include "SidePanel.h"

UTexture* SidePanel::RightPanelTexture = 0;

SidePanel::SidePanel( FVector2D size, FVector2D spacing ) :
  StackPanel( "SidePanel", RightPanelTexture, FLinearColor::White )
{
  Align = HAlign::Right | VAlign::VFull;
  XLimits = 0;
  YLimits = 0;
  Pad = spacing;

  Portraits = new PortraitsPanel( FVector2D( size.X, size.X ) );
  Stats = new StatsPanel();
  Actions = new ActionsPanel( "ActionsPanel", FVector2D( size.X/3, size.X/3 ) );
  minimap = new Minimap( 4.f, FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
  
  // Add the leftBorder in as last child, because it takes up full height,
  // and stackpanel will stack it in below the border
  leftBorder = new SolidWidget( "panel leftborder",
    FVector2D( 4.f, size.Y ), FLinearColor( 0.1f, 0.1f, 0.1f, 1.f ) );
  leftBorder->Margin = - Pad + FVector2D( -4.f, 0 );
  controls = new Controls();

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

  Restack();
}

void SidePanel::Set( vector<AGameObject*> objects )
{
  Portraits->Set( objects );
  Stats->Set( objects );
  Actions->Set( objects );
}

void SidePanel::Restack()
{
  children.clear();

  StackBottom( Portraits, HFull );
  StackBottom( Stats, HFull );
  StackBottom( Actions, HFull );
  StackBottom( minimap, HCenter );
  Add( leftBorder );
  Add( controls );
  Reflow();
}

void SidePanel::render( FVector2D offset )
{
  bool statsDirty = Stats->dirty;
  StackPanel::render( offset );
  // when the stats panel gets updated, before rendering it, we have to restack the elements
  if( statsDirty )
  {
    Restack();
    StackPanel::render( offset ); // re-render after re-stacking
  }
}


