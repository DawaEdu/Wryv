#include "Wryv.h"
#include "Shape.h"

AShape::AShape( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "mesh1" );
  Mesh->AttachTo( RootComponent );
}

void AShape::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  MaxDeadTime = 0.f;
  Grounds = 0; // Doesn't stick to the ground plane.
}

bool AShape::SetDestination( FVector d )
{
  // Overrides base so pathfinding isn't used.
  Dest = d;
  return 1;  // Always succeeds
}


