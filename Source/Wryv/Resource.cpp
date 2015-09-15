#include "Wryv.h"
#include "Resource.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "FlyCam.h"
#include "NotifyTreeFinishedFall.h"
#include "Peasant.h"

AResource::AResource( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, "mesh1" );
  Mesh->AttachTo( RootComponent );
  Jiggle = 0;
  AmountRemaining = 0.f;
}

void AResource::BeginPlay()
{
  Super::BeginPlay();
  AmountRemaining = Stats.Quantity;
}

void AResource::PostInitializeComponents()
{
  Super::PostInitializeComponents();
}

void AResource::Harvest( APeasant* peasant )
{
  // Just mined it. Get the resource.
  if( AmountRemaining > 0 )
  {
    float dmg = peasant->DamageRoll();
    peasant->MinedResources[ Stats.Type ] += dmg; // Use "damage" to determine mined qty
    AmountRemaining -= dmg;

    if( AmountRemaining <= 0 )
    {
      AmountRemaining = 0.f;
      ResourcesFinished = 1; // Signal for blueprints to kick-off the animation,
      // then, this flag is reset from blueprints
      Die();
    }
  }
}
