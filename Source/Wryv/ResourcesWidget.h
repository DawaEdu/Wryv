#pragma once

#include "StackPanel.h"

// Describes a gold, lumber, stone amount. Can be used for displaying player resources
// or the cost of something.
class ResourcesWidget : public StackPanel
{
  TextWidget *Gold, *Lumber, *Stone;
  int Px;       // size of the icons
  int Spacing;  // spacing between widgets
public:
  static UTexture *GoldTexture, *LumberTexture, *StoneTexture;

  // Add in all the subwidgets
  ResourcesWidget( FString name, int pxSize, int spacing ) :
    StackPanel( name, 0 ), Px( pxSize ), Spacing( spacing )
  {
    // +-----------------+
    // |G1000 W1000 S1000|
    // +-----------------+
    // The 3 resource types
    StackRight( new ImageWidget( "Gold icon", GoldTexture ) ); // icon
    Gold = new TextWidget( "1000" );
    StackRight( Gold );
    StackRight( new ImageWidget( "Lumber icon", LumberTexture ) );
    Lumber =  new TextWidget( "1000" );
    StackRight( Lumber );
    StackRight( new ImageWidget( "Stone icon", StoneTexture ) );
    Stone = new TextWidget( "1000" );
    StackRight( Stone );

    recomputeSizeToContainChildren();
  }
  virtual ~ResourcesWidget(){}
  void SetValues( int goldCost, int lumberCost, int stoneCost )
  {
    Gold->Set( goldCost );
    Lumber->Set( lumberCost );
    Stone->Set( stoneCost );
  }
};
