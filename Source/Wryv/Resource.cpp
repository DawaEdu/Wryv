#include "Wryv.h"
#include "FlyCam.h"
#include "Goldmine.h"
#include "NotifyTreeFinishedFall.h"
#include "Peasant.h"
#include "Resource.h"
#include "Stone.h"
#include "Tree.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

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
  AmountRemaining = Quantity;
}

void AResource::PostInitializeComponents()
{
  Super::PostInitializeComponents();
}

void AResource::Harvest( APeasant* peasant )
{
  // Just mined it. Get the resource.
  set< TSubclassOf<AResource> > acceptable = { 
    AGoldmine::StaticClass(), ATree::StaticClass(), AStone::StaticClass()
  };
  if( !IsAny( acceptable ) )
  {
    // This happens when you have a bad entry 
    error( FS( "Resource object type %s not recognized", *Stats.Name ) );
    return;
  }

  float dmg = peasant->DamageRoll();
  // Use "damage" to determine mined qty
  peasant->Mining = GetCPPClass();
  peasant->AddMined( peasant->Mining, dmg );
  AmountRemaining -= dmg;

  if( AmountRemaining <= 0 )
  {
    AmountRemaining = 0.f;
    ResourcesFinished = 1; // Signal for blueprints to kick-off the animation,
    // then, this flag is reset from blueprints

    // There's a reason this cannot go in ::Die(): When Die() is called the base
    // clears the Attackers array, but we need that array to search for new resources.
    // Tell each peasant harvesting peasant to go find a new resource
    vector<AGameObject*> Harvesters = Attackers; //Save old collection
    Die(); // Mark dead (clears Attackers group)
    //Re-target all attackers to find new resources
    for( AGameObject * go : Harvesters )
      if( APeasant *peasant = Cast<APeasant>( go ) )
        if( AResource* res = peasant->FindAndTargetNewResource( Pos, {peasant->Mining}, peasant->Stats.SightRange ) )
          peasant->Target( res );
  }
}


