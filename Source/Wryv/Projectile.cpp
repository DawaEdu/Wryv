#include "Wryv.h"
#include "Projectile.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

AProjectile::AProjectile(const FObjectInitializer& PCIP) : AGameObject(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
  AttackTarget = 0;
  Shooter = 0;
}

void AProjectile::BeginPlay()
{
  Super::BeginPlay();
}

void AProjectile::ai( float t )
{
  // The AI for the spell is basically nothing at this point,
  // though we might want to add spin/curl to some spells
}

void AProjectile::Move( float t )
{
  AGameObject::Move( t );
}

void AProjectile::SetDestinationArc( FVector start, FVector end, float height )
{
  // parabola eqn
  FVector dir = end - start;
  float len = dir.Size();
  float a = len / 2.f;
  float b = height;
  float c = b / (a*a);

  int N = 5;
  for( int i = 0; i <= N; i++ )
  {
    float t = i/N;
    float x = t*len;
    FVector v = FMath::Lerp( start, end, t );
    float xa = x-a;
    v.Z = -c*xa*xa + b;
    Waypoints.push_back( v );
  }
}
