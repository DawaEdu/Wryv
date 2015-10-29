#include "Wryv.h"

#include "Buffs.h"
#include "Building.h"
#include "ImageWidget.h"
#include "Goldmine.h"
#include "Peasant.h"
#include "Resource.h"
#include "StatsPanel.h"
#include "Stone.h"
#include "TextWidget.h"
#include "Tree.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

StatsPanel::StatsPanel() :
  StackPanel( "Stats", SolidWidget::SolidWhiteTexture, FLinearColor( 0.15, 0.15, 0.15, 0.15 ) )
{
  unitName = new TextWidget( "|" );
  unitName->Font = Game->hud->mediumFont;
  hpBar = new ProgressBar( FS( "HpBar %s", *Name ), 25.f, ProgressBar::Bkg, ProgressBar::Fill );
  buffs = new Buffs( "Buffs", 0 );
  hpText = new TextWidget( "|" );
  damage = new TextWidget( "|" );
  armor = new TextWidget( "|" );
  description = new TextWidget( "|" );
  resourcesCarrying = new ResourcesWidget( "ResourcesWidget", 16, 4 );
  Align = HFull | VCenter;
  BarSize = 25.f;

  Restack();
}

void StatsPanel::Blank()
{
  unitName -> Set( "" );
  buffs -> Set( AGameObject::Nothing ); // Clear buffs
  hpText -> Set( "" );
  damage -> Set( "" );
  armor -> Set( "" );
  description -> Set( "" );
  resourcesCarrying -> SetValues( 0, 0, 0 );
}

void StatsPanel::Restack()
{
  ClearChildren();
  Add( unitName );
  StackBottom( hpBar, HFull );
  StackBottom( hpText, HCenter );
  StackBottom( damage, HCenter );
  StackBottom( armor, HCenter );
  StackBottom( description, HCenter );
  StackBottom( resourcesCarrying, HCenter );
  Add( buffs );
  recomputeSizeToContainChildren();
}

void StatsPanel::Set( vector<AGameObject*> objects )
{
  Blank();  // blank the stats
  dirty = 1;
  if( !objects.size() ) {
    hpBar -> Size.Y = 0.f;
    return;
  }
  hpBar -> Size.Y = BarSize;
  
  AGameObject* go = objects.front();
  unitName->Set( go->Stats.Name );
  resourcesCarrying -> Hide(); // Hide unless peasant

  if( AResource* res = Cast<AResource>( go ) )
  {
    buffs -> Set( AGameObject::Nothing ); // Clear buffs
    hpBar -> Set( res->ResourcesFraction() ); // Instead of using HP, use amount of resources remaining
    hpText -> Set( FS( "Amount: %.0f / %d", res->AmountRemaining, res->Quantity ) );
    hpText -> Color = FLinearColor::LerpUsingHSV( FLinearColor(0.639f, 0.f, 0.192f), 
      FLinearColor(0.f, 0.7f, 0.f), res->ResourcesFraction() );
    // resources don't have dmg or armor
    damage -> Hide();
    armor -> Hide();
    description -> Set( res->Stats.Description );
  }
  else // some other type of gameobject,
  {
    buffs -> Set( go ); // Clear buffs
    hpBar -> Set( go->HpFraction() );
    hpText -> Set( FS( "Hp: %.0f / %.0f", go->Hp, go->Stats.HpMax ) );
    hpText -> Color = FLinearColor::LerpUsingHSV( FLinearColor(0.639f, 0.f, 0.192f), 
      FLinearColor(0.f, 0.7f, 0.f), go->HpFraction() );
    damage -> Set( FS( "Damage: %d", go->Stats.BaseAttackDamage ) );
    armor -> Set( FS( "Armor: %d", go->Stats.Armor ) );
    description -> Set( go->Stats.Description );
    if( APeasant* peasant = Cast<APeasant>( go ) )
    {
      // display MinedResources types
      int gold = peasant->MinedResources[ AGoldmine::StaticClass() ];
      int lumber = peasant->MinedResources[ ATree::StaticClass() ];
      int stone = peasant->MinedResources[ AStone::StaticClass() ];

      resourcesCarrying->SetValues( gold, lumber, stone );
      resourcesCarrying->Show();
    }
  }
}

void StatsPanel::render( FVector2D offset )
{
  StackPanel::render( offset ); // Render call happens here, so that children are re-measured

  if( dirty )
  {
    Restack();
    StackPanel::render( offset ); // re-render after re-stack
    dirty = 0;
  }

}

