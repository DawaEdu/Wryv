#include "Wryv.h"
#include "Shape.h"

AShape::AShape( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "mesh1" );
  Mesh->AttachTo( RootComponent );
}
