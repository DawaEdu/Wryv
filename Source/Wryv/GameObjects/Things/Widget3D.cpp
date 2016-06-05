#include "Wryv.h"
#include "Util/GlobalFunctions.h"
#include "GameObjects/Things/Widget3D.h"

AWidget3D::AWidget3D( const FObjectInitializer& PCIP ) : Super( PCIP )
{
}

void AWidget3D::BeginPlay()
{
  Super::BeginPlay();
}

void AWidget3D::Tick( float t )
{
  //LOG( "%s::Tick()", *GetName() );
  USceneComponent* rc = GetRootComponent();
  // Zero the relative transform, because the engine automatically
  // maintains the original position
  rc->SetRelativeLocation( FVector(0,0,0) );
  Super::Tick( t );
}