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
  // Using a popup for the UI information about the build costs
  // makes it so new players don't actually have to look for it.
  // It pops up, so you always notice it right away.
  CostWidget( UTexture* bkg ) : StackPanel( "CostWidget", bkg )
  {
    Pad = FVector2D(16,13);
    TopText = new TextWidget( "TopText" );
    TopText->Align = TopCenter;
    StackBottom( TopText );

    Cost = new ResourcesWidget( "CostWidget's ResourcesWidget", 16, 4 );
    Cost->Align = TopCenter;
    StackBottom(Cost);

    BottomText = new TextWidget( "BottomText" );
    BottomText->Align = TopCenter;
    StackBottom(BottomText);

    recomputeSizeToContainChildren();
  }
  virtual ~CostWidget(){}
  void Set( FString top, int goldCost, int lumberCost, int stoneCost, FString bottom )
  {
    TopText->Set( top );
    Cost->SetValues( goldCost, lumberCost, stoneCost );
    BottomText->Set( bottom );
    dirty = 1;
  }
};
