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
}

void AResource::BeginPlay()
{
  Super::BeginPlay();
  AmountRemaining = OriginalAmount;
}

void AResource::PostInitializeComponents()
{
  Super::PostInitializeComponents();
}

void AResource::Harvest( APeasant* peasant )
{
  // Just mined it. Get the resource.
  switch( Stats.Type )
  {
    case RESTREE:
      peasant->team->Lumber += Multiplier;
      break;
    case RESGOLDMINE:
      peasant->team->Gold += Multiplier;
      break;
    case RESSTONE:
      peasant->team->Stone += Multiplier;
      break;
    default:
      error( FS( "Resource type `%s` not a resource", *GetTypesName( Stats.Type ) ) );
      break;
  }

  // We've mined.
  if( AmountRemaining > 0 )
  {
    AmountRemaining--;

    if( !AmountRemaining )
    {
      ResourcesFinished = 1; // Signal for blueprints to kick-off the animation,
      // then, this flag is reset from blueprints
      Die();
    }
  }

  // Reset mining time. TimeLength is a polymorphic property.
  peasant->MiningTime = Stats.TimeLength;
}
