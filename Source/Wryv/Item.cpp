#include "Wryv.h"

#include "GlobalFunctions.h"
#include "Item.h"
#include "Unit.h"

// Sets default values
AItem::AItem(const FObjectInitializer& PCIP) : Super(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
}

void AItem::Hit( AGameObject* other )
{
  // Apply the powerup to the otheractor
  if( AUnit *unit = Cast<AUnit>( other ) )
  {
    LOG( "%s picked up a %s", *unit->Stats.Name, *Stats.Name );
    unit->CountersItems.push_back( NewObject<UItemAction>( unit, ButtonClass ) );
    Cleanup();
  }
}


