#include "Wryv.h"
#include "GroundPlane.h"

AGroundPlane::AGroundPlane( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "mesh1" );
  Mesh->AttachTo( RootComponent );

  TraceFraction = 1.f;
}

void AGroundPlane::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  Box = Mesh->Bounds.GetBox();
  TraceFraction = FMath::Clamp( TraceFraction, 0.25f, 0.85f );
}

FBox AGroundPlane::GetReducedBox()
{
  FBox box = Box;
  box.Max.X *= TraceFraction;
  box.Max.Y *= TraceFraction;
  box.Min.X *= TraceFraction;
  box.Min.Y *= TraceFraction;
  return box;
}

