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
 	// Set this actor to call Tick() every frame.
  // You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  AttackTarget = 0;
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

  //
  // << Consume the item, either by applying an effect to the consumer or spawning an effect in the world >>
  //

  // the `model` object of the player. Items[i] gets reflushed each frame.
  ApplyEffect( Items[i] );
  if( !Items[i].Quantity ) {
    Items[i].Type = Types::NOTHING;
    // strip from array
    Items.RemoveAt( i );
  }
}

void AUnit::Move( float t )
{
  Super::Move( t );

  // Tick all the traits
  for( int i = BonusTraits.size() - 1; i >= 0; i-- ) {
    BonusTraits[i].time -= t;
    if( BonusTraits[i].time <= 0 )
      BonusTraits.erase( BonusTraits.begin() + i );
  }
}


