#include "Wryv.h"
#include "GroundPlane.h"

AGroundPlane::AGroundPlane( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, TEXT( "Mesh" ) );
  Mesh->AttachTo( RootComponent );

  TraceFraction = 1.f;
}

void AGroundPlane::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  TraceFraction = FMath::Clamp( TraceFraction, 0.25f, 0.85f );
}

FBox AGroundPlane::GetBox()
{
  return GetComponentsBoundingBox();
}

FBox AGroundPlane::GetReducedBox()
{
  FBox ReducedBox = GetBox();
  ReducedBox.Max.X *= TraceFraction;
  ReducedBox.Max.Y *= TraceFraction;
  ReducedBox.Min.X *= TraceFraction;
  ReducedBox.Min.Y *= TraceFraction;
  return ReducedBox;
}

