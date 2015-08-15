#include "Wryv.h"
#include "StatsPanel.h"
#include "TextWidget.h"

StatsPanel::StatsPanel( FString iname, UTexture* tex, FLinearColor color ) :
  StackPanel( iname, tex, color )
{ 
  unitName = new TextWidget( "|" );
  StackBottom( unitName );
  hp = new TextWidget( "|" );
  StackBottom( hp );
  damage = new TextWidget( "|" );
  StackBottom( damage );
  armor = new TextWidget( "|" );
  StackBottom( armor );
  description = new TextWidget( "|" );
  StackBottom( description );
  Blank();
}

void StatsPanel::Blank()
{
  unitName -> Set( "" );
  hp -> Set( "" );
  damage -> Set( "" );
  armor -> Set( "" );
  description -> Set( "" );
}

void StatsPanel::Set( AGameObject* go )
{
  if( !go ) {
    Blank();  // blank the stats
    return;
  }
  
  // set the text inside with gameobject
  unitName->Set( go->Stats.Name );
  Add( unitName ); // RE-add, because text has changed
  hp -> Set( FS( "%.0f / %.0f", go->Hp, go->Stats.HpMax ) );
  hp -> Color = FLinearColor::LerpUsingHSV( FLinearColor::Red, FLinearColor::Green, go->hpPercent() );
  Add( hp );
  damage -> Set( FS( "Damage: %d", go->Stats.AttackDamage ) );
  Add( damage );
  armor -> Set( FS( "Armor: %d", go->Stats.Armor ) );
  Add( armor );
  description -> Set( go->Stats.Description );
  Add( description );
}
