#include "Wryv.h"

#include "GlobalFunctions.h"
#include "Item.h"
#include "Unit.h"

#include "ItemAction.h"

// Sets default values
AItem::AItem(const FObjectInitializer& PCIP) : Super(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;

  Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "itemsMesh" );
  Mesh->AttachTo( DummyRoot ); 
}

void AItem::Move( float t )
{
  // There is no movement code for an item,
  // and we don't invoke the base
  //AGameObject::Move( t );
}

void AItem::Hit( AGameObject* other )
{
  // Apply the powerup to the otheractor
  if( AUnit *unit = Cast<AUnit>( other ) )
  {
    LOG( "%s picked up %dx%s", *unit->Stats.Name, Quantity, *Stats.Name );
    unit->AddItem( ButtonClass );
    Die();
    Cleanup();
  }
}

void AItem::Die()
{
  AGameObject::Die();
}

