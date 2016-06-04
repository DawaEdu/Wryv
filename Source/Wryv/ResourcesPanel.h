#pragma once

#include "Cost.h"
#include "StackPanel.h"

// Describes a gold, lumber, stone amount. Can be used for displaying player resources
// or the cost of something.
class ResourcesPanel : public StackPanel
{
  TextWidget *Gold, *Lumber, *Stone;
  // This is the currently displayed amount of gold, lumber, stone
  // These are state variables since they are refreshed each frame.
  FCost resources, displayedResources;
  int Px;       // size of the icons
  int Spacing;  // spacing between widgets
public:
  static UTexture *GoldTexture, *LumberTexture, *StoneTexture;

  // Add in all the subwidgets
  ResourcesPanel( FString name, int pxSize, int spacing );
  virtual ~ResourcesPanel(){}
  void SetValues( FCost res );
  virtual void Update( float t );
};
