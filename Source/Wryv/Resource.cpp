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
  if( AmountRemaining <= 0 )
  {
    error( FS( "Peasant %s is mining an empty resource", *peasant->GetName() ) );
    return;
  }
  
  float dmg = peasant->DamageRoll();
  set<Types> acceptable = {RESGOLD,RESLUMBER,RESSTONE};
  if( !in( acceptable, Stats.Type.GetValue() ) )
  {
    // This happens when you have a bad entry in BaseStats.
    error( FS( "Resource object type %s not recognized", *Stats.Name ) );
    return;
  }

  peasant->Mining = Stats.Type;
  peasant->MinedResources[ Stats.Type ] += dmg; // Use "damage" to determine mined qty
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
    {
      if( APeasant *peasant = Cast<APeasant>( go ) )
      {
        if( AResource* res = peasant->FindNewResource( Pos, peasant->Mining, peasant->Stats.SightRange ) )
          peasant->Target( res );
      }
    }
  }
}


