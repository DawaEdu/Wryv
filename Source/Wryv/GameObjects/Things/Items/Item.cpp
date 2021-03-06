#include "Wryv.h"

#include "Util/GlobalFunctions.h"
#include "Item.h"
#include "GameObjects/Units/Unit.h"

#include "UI/UICommand/Command/UIItemActionCommand.h"

// Sets default values
AItem::AItem( const FObjectInitializer& PCIP ) : Super(PCIP)
{
  Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, TEXT( "ItemsMesh" ) );
  Mesh->AttachTo( DummyRoot ); 
}

void AItem::Move( float t )
{
  // There is no movement code for an item,
  // and we don't invoke the base
  AGameObject::Move( t );
}

void AItem::Pickup( AGameObject* other )
{
  // Apply the powerup to the otheractor
  if( AUnit *unit = Cast<AUnit>( other ) )
  {
    if( !ItemActionClassAndQuantity.ItemActionClass )
    {
      error( FS( "ItemActionClassAndQuantity not set" ) );
      return;
    }

    LOG( "%s picked up %dx%s, action=%s", *unit->Stats.Name,
      ItemActionClassAndQuantity.Quantity, *Stats.Name,
      *ItemActionClassAndQuantity.ItemActionClass->GetName() );
    unit->AddItem( ItemActionClassAndQuantity );
    Die();
    Cleanup();
  }
}

void AItem::Die()
{
  AGameObject::Die();
}

