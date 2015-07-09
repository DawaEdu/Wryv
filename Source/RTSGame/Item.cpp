#include "RTSGame.h"
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
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>( this );
  //UE_LOG( LogTemp, Warning, TEXT("meshes %d"), meshes.size() );

  // Connect the Item's meshes to a proximity function.
  for( UMeshComponent* m : meshes )
  {
    m->OnComponentBeginOverlap.AddDynamic( this, &AItem::ProxPickup );
    UE_LOG( LogTemp, Warning, TEXT("Registering item prox pickup %s (%s)"),
      *UnitsData.Name, *m->GetName() );
  }
}

void AItem::ProxPickup_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult )
{
  // Apply the powerup to the otheractor
  if( AUnit *unit = Cast<AUnit>( OtherActor ) )
  {
    unit->Items.Push( UnitsData.Type );
    UE_LOG( LogTemp, Warning, TEXT("%s picked up a %s"),
      *unit->UnitsData.Name, *UnitsData.Name );
    Destroy();
  }

}


