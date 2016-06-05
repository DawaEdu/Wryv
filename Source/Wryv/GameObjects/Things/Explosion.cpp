#include "Wryv.h"  // PCH
#include "GameObjects/Things/Explosion.h"

AExplosion::AExplosion( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  explosion = PCIP.CreateDefaultSubobject<UParticleSystemComponent>( this, TEXT( "Particles" ) );
  explosion->AttachTo( RootComponent );

  Untargettable = 1;
}

void AExplosion::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  explosion->OnSystemFinished.AddDynamic( this, &AExplosion::OnEmitterFinished );
}

void AExplosion::OnEmitterFinished_Implementation( UParticleSystemComponent* PSystem )
{
  LOG( "Emitter finished" );
  Die();
  Cleanup();
}
