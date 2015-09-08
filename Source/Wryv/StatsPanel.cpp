#include "Wryv.h"
#include "StatsPanel.h"
#include "TextWidget.h"
#include "Building.h"

StatsPanel::StatsPanel() :
  StackPanel( "Stats", SolidWidget::SolidWhiteTexture, FLinearColor( 0.15, 0.15, 0.15, 0.15 ) )
{ 
  unitName = new TextWidget( "|" );
  hpBar = new ProgressBar( "HpBar", 25.f,
    FLinearColor(0.655, 0.000, 0.125, 1.f), //Purplish-red
    FLinearColor(0.169, 0.796, 0.278, 1.f) );
  hpText = new TextWidget( "|" );
  damage = new TextWidget( "|" );
  armor = new TextWidget( "|" );
  description = new TextWidget( "|" );
  Selected = 0;
  Align = HFull | VCenter;

  Restack();
}

void StatsPanel::Blank()
{
  unitName -> Set( "" );
  hpBar->Set( 0.f );
  hpText -> Set( "" );
  damage -> Set( "" );
  armor -> Set( "" );
  description -> Set( "" );
}

void StatsPanel::Restack()
{
  children.clear();

  Add( unitName );
  StackBottom( hpBar, HFull );
  StackBottom( hpText, HCenter );
  StackBottom( damage, HCenter );
  StackBottom( armor, HCenter );
  StackBottom( description, HCenter );
}

void StatsPanel::Set( AGameObject* go )
{
  Selected = go;
  Blank();  // blank the stats
  Restack();
  if( !go )  return;
  
  // set the text inside with gameobject
  unitName->Set( go->Stats.Name );
  hpBar->Set( go->HpPercent() );

  hpText -> Set( FS( "%.0f / %.0f", go->Hp, go->Stats.HpMax ) );
  hpText -> Color = FLinearColor::LerpUsingHSV( FLinearColor(0.639f, 0.f, 0.192f), 
    FLinearColor(0.f, 0.7f, 0.f), go->HpPercent() );
  damage -> Set( FS( "Damage: %d", go->Stats.BaseAttackDamage ) );
  armor -> Set( FS( "Armor: %d", go->Stats.Armor ) );
  description -> Set( go->Stats.Description );

  Restack();
}

void StatsPanel::Move( float t )
{
  StackPanel::Move( t );
  
  Set( Selected );
}

