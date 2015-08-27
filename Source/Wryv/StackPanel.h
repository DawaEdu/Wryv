#pragma once

#include "ImageWidget.h"

// Supports stacking-in of widgets from left/right or top/bottom
class StackPanel : public ImageWidget
{
public:
  static UTexture* StackPanelTexture;
  enum StackMode {
    Center,Full
  };
  
  StackPanel( FString name );
  StackPanel( FString name, UTexture* bkg );
  StackPanel( FString name, UTexture* bkg, FLinearColor color );
  virtual ~StackPanel(){}
  
  // Override base class function, to prevent warning when
  // texture not provided
  virtual void render( FVector2D offset ) override;

  //    _ _
  // > |_|_|
  template <typename T> T* StackLeft( HotSpot* widget, VAlign vMode )
  {
    widget->Align = Left | vMode;
    // push all the children already in the widget over to the right
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->Margin.X += widget->Size.X + Pad.X;
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  // vMode is either VCentering or VFull
  //  _ _
  // |_|_| <
  template <typename T> T* StackRight( T* widget, VAlign vMode )
  {
    widget->Align = Left | vMode;
    
    // Situates the bounds @ the origin,
    // which means its just a measure of the widget's total size.
    FBox2DU childBounds = GetChildAbsBounds();
    widget->Margin.X = childBounds.Size().X;
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  //  v
  //  _
  // |_|
  // |_|
  template <typename T> T* StackTop( T* widget, HAlign hMode )
  {
    widget->Align = Top | hMode;
    for( int i = 0 ; i < children.size(); i++ )
      children[i]->Margin.Y += widget->Size.Y + Pad.Y;
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }

  //  _
  // |_|
  // |_|
  //  ^
  template <typename T> T* StackBottom( T* widget, HAlign hMode )
  {
    widget->Align = Top | hMode;
    FBox2DU childBounds = GetChildAbsBounds();
    widget->Margin.Y = childBounds.Size().Y;
    Add( widget );
    recomputeSizeToContainChildren();
    return widget;
  }
};


