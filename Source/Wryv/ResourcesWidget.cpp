#include "Wryv.h"

#include "ResourcesWidget.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

UTexture* ResourcesWidget::GoldTexture = 0;
UTexture* ResourcesWidget::LumberTexture = 0;
UTexture* ResourcesWidget::StoneTexture = 0;

ResourcesWidget::ResourcesWidget( FString name, int pxSize, int spacing ) :
  StackPanel( name, 0 ), Px( pxSize ), Spacing( spacing )
{
  displayedGold = displayedLumber = displayedStone = 0.f;

  // +-----------------+
  // |G1000 W1000 S1000|
  // +-----------------+
  // The 3 resource types
  StackRight( new ImageWidget( "Gold icon", GoldTexture ), VCenter ); // icon
  StackRight( Gold = new TextWidget( "1000" ), VCenter );
  StackRight( new ImageWidget( "Lumber icon", LumberTexture ), VCenter );
  StackRight( Lumber = new TextWidget( "1000" ), VCenter );
  StackRight( new ImageWidget( "Stone icon", StoneTexture ), VCenter );
  StackRight( Stone = new TextWidget( "1000" ), VCenter );

  recomputeSizeToContainChildren();
}

void ResourcesWidget::SetValues( int goldCost, int lumberCost, int stoneCost )
{
  Gold->Set( goldCost );
  Lumber->Set( lumberCost );
  Stone->Set( stoneCost );
}

void ResourcesWidget::Update( float t )
{
  // spent=200, diff=-200
  //    diff = 200 - 400;
  float diff = Game->gm->playersTeam->Gold - displayedGold;
  diff *= 0.1; // jump by 10%
  displayedGold += diff;

  diff = Game->gm->playersTeam->Lumber - displayedLumber;
  diff *= 0.1; // jump by 10%
  displayedLumber += diff;

  diff = Game->gm->playersTeam->Stone - displayedStone;
  diff *= 0.1; // jump by 10%
  displayedStone += diff;

  SetValues( FMath::RoundToInt( displayedGold ),
    FMath::RoundToInt( displayedLumber ),
    FMath::RoundToInt( displayedStone ) );
}


