#include "Wryv.h"
#include "CostWidget.h"

UTexture* CostWidget::CostWidgetBackground = 0;

CostWidget::CostWidget() : StackPanel( "CostWidget", CostWidgetBackground )
{
  Pad = FVector2D(16, 13);

  TopText = new TextWidget( "TopText" );
  StackBottom( TopText, HCenter );

  Cost = new ResourcesWidget( "CostWidget's ResourcesWidget", 16, 4 );
  StackBottom( Cost, HCenter );

  BottomText = new TextWidget( "BottomText" );
  StackBottom( BottomText, HCenter );

  recomputeSizeToContainChildren();
}

void CostWidget::Set( FString top, int goldCost, int lumberCost, int stoneCost, FString bottom )
{
  TopText->Set( top );
  Cost->SetValues( goldCost, lumberCost, stoneCost );
  BottomText->Set( bottom );
  dirty = 1;
}


