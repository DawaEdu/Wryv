#include "Wryv.h"
#include "CostWidget.h"

UTexture* CostWidget::CostWidgetBackground = 0;

CostWidget::CostWidget() : StackPanel( "CostWidget", CostWidgetBackground )
{
  Pad = FVector2D(16, 13);

  TopText = new TextWidget( "TopText" );
  TopText->Align = TopCenter;
  StackBottom( TopText );

  Cost = new ResourcesWidget( "CostWidget's ResourcesWidget", 16, 4 );
  Cost->Align = TopCenter;
  StackBottom( Cost );

  BottomText = new TextWidget( "BottomText" );
  BottomText->Align = TopCenter;
  StackBottom( BottomText );

  recomputeSizeToContainChildren();
}

void CostWidget::Set( FString top, int goldCost, int lumberCost, int stoneCost, FString bottom )
{
  TopText->Set( top );
  Cost->SetValues( goldCost, lumberCost, stoneCost );
  BottomText->Set( bottom );
  dirty = 1;
}


