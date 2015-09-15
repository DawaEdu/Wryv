#include "Wryv.h"  // PCH
#include "Explosion.h"

AExplosion::AExplosion( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  explosion = PCIP.CreateDefaultSubobject<UParticleSystemComponent>( this, "Particles" );
  explosion->AttachTo( RootComponent );
}

void AExplosion::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  explosion->OnSystemFinished.AddDynamic( this, &AExplosion::OnEmitterFinished );
}

void AExplosion::OnEmitterFinished_Implementation( UParticleSystemComponent* PSystem )
{
  LOG( "Emitter finished" );
  Cleanup();
}
