#pragma once

#include "Util/Cost.h"
#include "UI/HotSpot/Panels/StackPanel.h"
#include "UI/HotSpot/GamePanels/ResourcesPanel.h"

// Describes the cost of something
// +-------------------------------------+
// | Barracks                            |
// | [] gold [] lumber [] stone          |
// | Barracks are used to build militia. |
// +-------------------------------------+
class CostPanel : public StackPanel
{
  TextWidget* TopText;
  ResourcesPanel* Cost;
  TextWidget* BottomText;
public:
  static UTexture* CostWidgetBackground;
  // Using a popup for the UI information about the build costs
  // makes it so new players don't actually have to look for it.
  // It pops up, so you always notice it right away.
  CostPanel();
  virtual ~CostPanel(){}
  void Set( FString top, FCost costs, FString bottom );
};

