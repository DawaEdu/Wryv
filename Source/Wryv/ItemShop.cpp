#include "Wryv.h"
#include "ItemShop.h"
#include "WryvGameInstance.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Unit.h"

AItemShop::AItemShop( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  patron = 0;
  sphere = 0;
}

void AItemShop::BeginPlay()
{
  Super::BeginPlay();
  // Connect the Item's meshes to a proximity function.
  sphere = GetComponentByType<USphereComponent>( this ); //GetComponentByName<USphereComponent>( this, "sphere" );
  if( sphere )
  {
    sphere->OnComponentBeginOverlap.AddDynamic( this, &AItemShop::Prox );
    sphere->OnComponentEndOverlap.AddDynamic( this, &AItemShop::ProxEnd );
    LOG(  "Sphere AItemShop::Prox() registered %s",
      *sphere->GetName() );
  }
  else
  {
    LOG(  "ERROR: ItemShop has no bounding sphere");
  }
}

void AItemShop::Prox_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp,
  int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult )
{
  LOG(  "ItemShop's prox patron = %s", *OtherActor->GetName() );

  // When you go near an item shop, clicking it makes its items available.
  patron = (AUnit*)OtherActor;
}

void AItemShop::ProxEnd_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
  LOG(  "proxend %s", *OtherActor->GetName() );

  // if this is the object that was the pattern, unselect as patron, then
  // try to select another patron from other units this player owns
  if( patron == OtherActor )
  {
    // Sweep for another thing within the prox sphere to act as patron, else, set to NULL.
    TArray<AActor*> actors;
    sphere->GetOverlappingActors( actors );
      
    // check to see if there are any overlapping actors that are Unit* objects
    for( int i = 0; i < actors.Num(); i++ )
    {
      AActor *a = actors[i];
      if( AUnit *u = Cast<AUnit>( a ) )
      {
        patron = u;
        LOG(  "%s exited prox, Unit %s proxed itemshop",
          *OtherActor->GetName(), *a->GetName() );
        return;
      }
    }  
  }
    
  LOG(  "no other actor proxed itemshop");
  patron = 0;
}
