#include "Wryv.h"

#include "UE4/Flycam.h"
#include "GameObjects/Things/Resources/Goldmine.h"
#include "UE4/NotifyTreeFinishedFall.h"
#include "GameObjects/Units/Peasant.h"
#include "GameObjects/Things/Resources/Resource.h"
#include "GameObjects/Things/Resources/Stone.h"
#include "UE4/TheHUD.h"
#include "GameObjects/Things/Resources/Tree.h"
#include "UE4/WryvGameInstance.h"
#include "UE4/WryvGameMode.h"

AResource::AResource( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, TEXT( "Mesh" ) );
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

  // Start jiggling.
  Jiggle = 1;

  UClass* resourceClass = GetClass();
  if( resourceClass->IsChildOf<AGoldmine>() )
    resourceClass = AGoldmine::StaticClass();
  else if( resourceClass->IsChildOf<ATree>() )
    resourceClass = ATree::StaticClass();
  else if( resourceClass->IsChildOf<AStone>() ) 
    resourceClass = AStone::StaticClass();
  //info( FS( "%s is a %s", *GetName(), *resourceClass->GetName() ) );
  peasant->Mining = resourceClass;
  
  // Use "damage" to determine mined qty
  int mined = FMath::RoundToInt( peasant->DamageRoll() );
  peasant->AddMined( peasant->Mining, mined );
  AmountRemaining -= mined;
  Game->hud->ui->dirty = 1;

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
      if( APeasant *p = Cast<APeasant>( go ) )
        if( AResource* res = p->FindAndTargetNewResource( Pos, {p->Mining}, p->Stats.SightRange ) )
          peasant->Target( res );
  }
}


