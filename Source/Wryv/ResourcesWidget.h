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
  ResourcesWidget( FString name, int pxSize, int spacing );
  virtual ~ResourcesWidget(){}
  void SetValues( int goldCost, int lumberCost, int stoneCost );
};
