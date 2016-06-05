#include "Wryv.h"
#include "UI/HotSpot/GamePanels/CostPanel.h"

UTexture* CostPanel::CostWidgetBackground = 0;

CostPanel::CostPanel() : StackPanel( "CostPanel", CostWidgetBackground )
{
  Pad = FVector2D(16, 13);

  TopText = new TextWidget( "TopText" );
  StackBottom( TopText, HCenter );

  Cost = new ResourcesPanel( "CostPanel's ResourcesPanel", 16, 4 );
  StackBottom( Cost, HCenter );

  BottomText = new TextWidget( "BottomText" );
  StackBottom( BottomText, HCenter );

  recomputeSizeToContainChildren();
}

void CostPanel::Set( FString top, FCost costs, FString bottom )
{
  TopText->Set( top );
  Cost->SetValues( costs );
  BottomText->Set( bottom );
  dirty = 1;
}


