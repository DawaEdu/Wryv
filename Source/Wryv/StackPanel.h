#pragma once

#include "ImageWidget.h"

// Supports stacking-in of widgets from left/right or top/bottom
class StackPanel : public ImageWidget
{
public:
  static UTexture* StackPanelTexture;
  
  StackPanel( FString name ) : ImageWidget( name )
  {
  }
  StackPanel( FString name, UTexture* bkg ) : ImageWidget( name, bkg )
  {
  }
  StackPanel( FString name, UTexture* bkg, FLinearColor color ) : ImageWidget( name, bkg, color )
  {
  }
  virtual ~StackPanel(){}
  
  // Override base class function, to prevent warning when
  // texture not provided
  virtual void render( FVector2D offset ) override
  {
    // Doesn't render the texture if it isn't set,
    // so that clear background is allowed
    if( Tex )  ImageWidget::render( offset );
    // render children
    HotSpot::render( offset );
  }

  //  _ _
  // |_|_| <
  template <typename T> T* StackRight( T* widget )
  {
    widget->Align = CenterLeft;
    // Situates the bounds @ the origin,
    // which means its just a measure of the widget's total size.
    FBox2DU childBounds = GetChildBounds();
    widget->Margin.X = childBounds.right();
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  //  _
  // |_|
  // |_|
  //  ^
  template <typename T> T* StackBottom( T* widget )
  {
    widget->Align = TopCenter;
    FBox2DU childBounds = GetChildBounds();
    widget->Margin.Y = childBounds.bottom();
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  //    _ _
  // > |_|_|
  template <typename T> T* StackLeft( HotSpot* widget )
  {
    widget->Align = CenterLeft;
    // pull all the children already in the widget over to the left
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->Margin.X += widget->Size.X + Pad.X;
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  //  v
  //  _
  // |_|
  // |_|
  template <typename T> T* StackTop( T* widget )
  {
    widget->Align = TopCenter;
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->Margin.Y += widget->Size.Y + Pad.Y;
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }
};


