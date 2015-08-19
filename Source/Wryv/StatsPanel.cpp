#include "Wryv.h"
#include "StatsPanel.h"
#include "TextWidget.h"
#include "Building.h"

StatsPanel::StatsPanel( FString iname, UTexture* tex, FLinearColor color ) :
  StackPanel( iname, tex, color )
{ 
  unitName = new TextWidget( "|" );
  StackBottom( unitName );
  progress = new ProgressBar( "Progress bar", FLinearColor::Red, FLinearColor::Green, FVector2D( Size.X*0.8, 50.f ) );
  StackBottom( progress );
  hp = new TextWidget( "|" );
  StackBottom( hp );
  damage = new TextWidget( "|" );
  StackBottom( damage );
  armor = new TextWidget( "|" );
  StackBottom( armor );
  description = new TextWidget( "|" );
  StackBottom( description );
  Selected = 0;
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

void StatsPanel::Restack()
{
  StackBottom( unitName );
  StackBottom( progress );
  StackBottom( hp );
  StackBottom( damage );
  StackBottom( armor );
  StackBottom( description );
}

void StatsPanel::Set( AGameObject* go )
{
  Selected = go;
  if( !go ) {
    Blank();  // blank the stats
    return;
  }
  
  // set the text inside with gameobject
  unitName->Set( go->Stats.Name );
  Add( unitName ); // RE-add, because text has changed

  if( go->isBuilding() )
  {
    ABuilding* building = Cast<ABuilding>( go );
    progress->Set( building->percentBuilt() );
    progress->Show();
  }
  else progress->Hide();

  hp -> Set( FS( "%.0f / %.0f", go->Hp, go->Stats.HpMax ) );
  hp -> Color = FLinearColor::LerpUsingHSV( FLinearColor::Red, FLinearColor::Green, go->hpPercent() );
  Add( hp );
  damage -> Set( FS( "Damage: %d", go->Stats.AttackDamage ) );
  Add( damage );
  armor -> Set( FS( "Armor: %d", go->Stats.Armor ) );
  Add( armor );
  description -> Set( go->Stats.Description );
  Add( description );

  Restack();
}

void StatsPanel::Move( float t )
{
  StackPanel::Move( t );
  Set( Selected );
}

