#include "Wryv.h"
#include "GlobalFunctions.h"
#include "Item.h"
#include "Unit.h"

// Sets default values
AItem::AItem(const FObjectInitializer& PCIP) : Super(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
}

void AItem::BeginPlay()
{
	Super::BeginPlay();
   
  // Collision with the mesh picks up the item.
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>();
  //LOG( "meshes %d", meshes.size() );

  // Connect the Item's meshes to a proximity function.
  for( UMeshComponent* m : meshes )
  {
    m->OnComponentBeginOverlap.AddDynamic( this, &AItem::ProxPickup );
    LOG( "Registering item prox pickup %s (%s)",
      *Stats.Name, *m->GetName() );
  }
}

void AItem::ProxPickup_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult )
{
  // Apply the powerup to the otheractor
  if( AUnit *unit = Cast<AUnit>( OtherActor ) )
  {
    unit->Items.Push( Stats );
    LOG( "%s picked up a %s", *unit->Stats.Name, *Stats.Name );
    Cleanup();
  }

}


