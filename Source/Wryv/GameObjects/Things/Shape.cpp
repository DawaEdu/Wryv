#include "Wryv.h"
#include "GameObjects/Things/Shape.h"

AShape::AShape( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, TEXT( "Mesh" ) );
  Mesh->AttachTo( RootComponent );

  Untargettable = 1;
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


