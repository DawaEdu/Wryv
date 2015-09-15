#include "Wryv.h"
#include "Unit.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "PlayerControl.h"
#include "TheHUD.h"
#include "Widget.h"
#include "UnitsData.h"

#include <map>
using namespace std;

// Sets default values
AUnit::AUnit( const FObjectInitializer& PCIP ) : AGameObject( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, "themeshof" );
  Mesh->AttachTo( DummyRoot );
}

void AUnit::BeginPlay()
{
  Super::BeginPlay();
  for( int i = 0; i < StartingItems.Num(); i++ )
  {
    Items.Push( Game->unitsData[ StartingItems[i] ] );
  }
}

void AUnit::ConsumeItem( int i )
{
  if( i < 0 || i >= Items.Num() )
  {
    LOG( "%s cannot consume item %d / %d, OOR", *Stats.Name, i, Items.Num() );
    return;
  }

  // use the item. qty goes down by 1
  Items[i].Quantity--; // we don't affect the UI here, only

  // the `model` object of the player. Items[i] gets reflushed each frame.
  AddBuff( Items[i].Type );
  if( !Items[i].Quantity )
    Items.RemoveAt( i );
  
}

void AUnit::Move( float t )
{
  // recompute path
  if( Stats.SpeedMax )
  {
    if( FollowTarget )
    {
      MoveWithinDistanceOf( FollowTarget,  FollowTarget->Radius() );
    }
    else if( AttackTarget )
    {
      MoveWithinDistanceOf( AttackTarget, Stats.AttackRange * 0.9f );
      Face( AttackTarget->Pos );
    }
  }

  Walk( t );   // Walk towards destination
  AGameObject::Move( t );
}


