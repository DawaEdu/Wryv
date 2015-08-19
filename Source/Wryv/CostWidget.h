#pragma once

#include "StackPanel.h"
#include "ResourcesWidget.h"

// Describes the cost of something
// +-------------------------------------+
// | Barracks                            |
// | [] gold [] lumber [] stone          |
// | Barracks are used to build militia. |
// +-------------------------------------+
class CostWidget : public StackPanel
{
  TextWidget* TopText;
  ResourcesWidget* Cost;
  TextWidget* BottomText;
public:
  static UTexture* CostWidgetBackground;
  // Using a popup for the UI information about the build costs
  // makes it so new players don't actually have to look for it.
  // It pops up, so you always notice it right away.
  CostWidget();
  virtual ~CostWidget(){}
  void Set( FString top, int goldCost, int lumberCost, int stoneCost, FString bottom );
};

