#pragma once

#include "StackPanel.h"

// Describes a gold, lumber, stone amount. Can be used for displaying player resources
// or the cost of something.
class ResourcesWidget : public StackPanel
{
  TextWidget *Gold, *Lumber, *Stone;
  // This is the currently displayed amount of gold,lumber,stone
  // These are state variables since they are refreshed each frame.
  float displayedGold, displayedLumber, displayedStone;
  int Px;       // size of the icons
  int Spacing;  // spacing between widgets
public:
  static UTexture *GoldTexture, *LumberTexture, *StoneTexture;

  // Add in all the subwidgets
  ResourcesWidget( FString name, int pxSize, int spacing );
  virtual ~ResourcesWidget(){}
  void SetValues( int goldCost, int lumberCost, int stoneCost );
  virtual void Update( float t );
};
