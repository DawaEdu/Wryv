#include "Wryv.h"
#include "StackPanel.h"

UTexture* StackPanel::StackPanelTexture = 0;

StackPanel::StackPanel( FString name ) : ImageWidget( name )
{
  restacking = 0;
}
StackPanel::StackPanel( FString name, UTexture* bkg ) : ImageWidget( name, bkg )
{
  restacking = 0;
}
StackPanel::StackPanel( FString name, UTexture* bkg, FLinearColor color ) : ImageWidget( name, bkg, color )
{
  restacking = 0;
}

void StackPanel::Clear()
{
  stackOrder.clear();
  ImageWidget::Clear();
}

// Override base class function, to prevent warning when
// texture not provided
void StackPanel::render( FVector2D offset )
{
  if( hidden ) return;

  // Doesn't render the texture if it isn't set,
  // so that clear background is allowed
  if( Tex )  ImageWidget::render( offset );  //!! If remove NoTextureTexture, can remove this

  // render children
  HotSpot::render( offset );
}

void StackPanel::Reflow()
{
  if( restacking )
    return;
  restacking = 1;
  Restack();
  HotSpot::Reflow();
  restacking = 0;
}

void StackPanel::Restack()
{
  // Re-add all children in the mode they were added before.
  children.clear();

  for( int i = 0 ; i < stackOrder.size() ; i++ )
  {
    HotSpot* hs = stackOrder[i].hotspot;
    int align = stackOrder[i].Align;
    switch( stackOrder[i].Side )
    {
      case Stacking::StackSide::Left:
        StackLeft( hs, (VAlign)align );
        break;
      case Stacking::StackSide::Right:
        StackRight( hs, (VAlign)align );
        break;
      case Stacking::StackSide::Bottom:
        StackBottom( hs, (HAlign)align );
        break;
      case Stacking::StackSide::Top:
        StackTop( hs, (HAlign)align );
        break;
      case Stacking::StackSide::None:
        Add( hs );
        break;
    }
  }
}

HotSpot* StackPanel::Add( HotSpot* w )
{
  HotSpot::Add( w );
  if( !restacking )
    stackOrder.push_back( Stacking( w, Stacking::StackSide::None, 0 ) );
  return w;
}

HotSpot* StackPanel::StackLeft( HotSpot* widget, VAlign vMode )
{
  widget->Align = Left | vMode;
  // push all the children already in the widget over to the right
  for( int i = 0 ; i < children.size(); i++ )
    children[i]->Margin.X += widget->Size.X + Pad.X;
  HotSpot::Add( widget );
  recomputeSizeToContainChildren();
  if( !restacking )
    stackOrder.push_back( Stacking( widget, Stacking::Left, vMode ) );
  return widget;
}

// vMode is either VCentering or VFull
//  _ _
// |_|_| <
HotSpot* StackPanel::StackRight( HotSpot* widget, VAlign vMode )
{
  widget->Align = Left | vMode; // VCenter|VFull
    
  // Situates the bounds @ the origin,
  // which means its just a measure of the widget's total size.
  FBox2DU childBounds = GetChildAbsBounds();
  widget->Margin.X = childBounds.Size().X;
  HotSpot::Add( widget );
  recomputeSizeToContainChildren();
  if( !restacking )
    stackOrder.push_back( Stacking( widget, Stacking::Right, vMode ) );
  return widget;
}

//  v
//  _
// |_|
// |_|
HotSpot* StackPanel::StackTop( HotSpot* widget, HAlign hMode )
{
  widget->Align = Top | hMode;
  for( int i = 0 ; i < children.size(); i++ )
    children[i]->Margin.Y += widget->Size.Y + Pad.Y;
  HotSpot::Add( widget );
  recomputeSizeToContainChildren();
  if( !restacking )
    stackOrder.push_back( Stacking( widget, Stacking::Top, hMode ) );
  return widget;
}

//  _
// |_|
// |_|
//  ^
HotSpot* StackPanel::StackBottom( HotSpot* widget, HAlign hMode )
{
  widget->Align = Top | hMode;
  FBox2DU childBounds = GetChildAbsBounds();
  widget->Margin.Y = childBounds.Size().Y;
  HotSpot::Add( widget );
  recomputeSizeToContainChildren();
  if( !restacking )
    stackOrder.push_back( Stacking( widget, Stacking::Bottom, hMode ) );
  return widget;
}


