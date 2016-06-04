#include "Wryv.h"
#include "ExplodableResource.h"

AExplodableResource::AExplodableResource( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  destructableMesh = PCIP.CreateDefaultSubobject<UDestructibleComponent>( this, TEXT( "DestructibleMesh1" ) );
  destructableMesh->AttachTo( DummyRoot );
  ExplosiveRadius = 3.f;
  ExplosiveForce = 10000.f;
}

void AExplodableResource::BeginPlay()
{
  Super::BeginPlay();
  MaxDeadTime = MaxExplosionTime;
}

void AExplodableResource::Die()
{
  Mesh->SetVisibility( false );
  destructableMesh->SetVisibility( true );
  destructableMesh->ApplyRadiusDamage( 111, Pos, ExplosiveRadius, ExplosiveForce, 1 ); // Shatter the destructable.

  AResource::Die();
}

