#pragma once

#include "UI/HotSpot/Elements/ImageHS.h"

struct Stacking
{
  enum StackSide{ None, Left, Right, Top, Bottom };
  HotSpot* hotspot;
  int Side;
  int Align;
  Stacking( HotSpot* hs, StackSide side, int align ) :
    hotspot( hs ), Side( side ), Align( align ) { }
};

// Supports stacking-in of widgets from left/right or top/bottom
class StackPanel : public ImageHS
{
public:
  static UTexture* StackPanelTexture;
  bool restacking; // On when reflow is occurring
  // Remember the stack order
  vector< Stacking > stackOrder;
  StackPanel( FString name );
  StackPanel( FString name, UTexture* bkg );
  StackPanel( FString name, UTexture* bkg, FLinearColor color );
  virtual ~StackPanel(){}
  
  virtual void Clear();
    // Override base class function, to prevent warning when
  // texture not provided
  virtual void render( FVector2D offset ) override;

  virtual void Reflow() override;

  virtual void Restack();

  virtual HotSpot* Add( HotSpot* w );

  //    _ _
  // > |_|_|
  HotSpot* StackLeft( HotSpot* widget, VAlign vMode );

  // vMode is either VCentering or VFull
  //  _ _
  // |_|_| <
  HotSpot* StackRight( HotSpot* widget, VAlign vMode );

  //  v
  //  _
  // |_|
  // |_|
  HotSpot* StackTop( HotSpot* widget, HAlign hMode );

  //  _
  // |_|
  // |_|
  //  ^
  HotSpot* StackBottom( HotSpot* widget, HAlign hMode );
};


