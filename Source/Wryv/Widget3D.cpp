#include "Wryv.h"
#include "Widget3D.h"
#include "GlobalFunctions.h"

AWidget3D::AWidget3D( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  PrimaryActorTick.bCanEverTick = true;
}

void AWidget3D::BeginPlay()
{
  Super::BeginPlay();
}

void AWidget3D::Tick( float t )
{
  //LOG( "%s::Tick()", *GetName() );
  USceneComponent* rc = GetRootComponent();
  //Print( TEXT("GetComponentTransform().GetLocation()"), rc->GetComponentTransform().GetLocation() );
  //Print( TEXT("GetRelativeTransform().GetLocation()"), rc->GetRelativeTransform().GetLocation() );
  // Zero the relative transform, because the engine automatically
  // maintains the original position
  rc->SetRelativeLocation( FVector(0,0,0) );
  Super::Tick( t );
}