#include "Wryv.h"
#include "Projectile.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "GlobalFunctions.h"
#include "FlyCam.h"

AProjectile::AProjectile(const FObjectInitializer& PCIP) : AGameObject(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
  Shooter = 0;
  Gravity = -10.f;
}

void AProjectile::ai( float t )
{
  // The AI for the spell is basically nothing at this point,
  // though we might want to add spin/curl to some spells
}

void AProjectile::Move( float t )
{
  AGameObject::Move( t ); // OVERRIDES BASE

  Vel.Z += Gravity * t;
  Pos += Vel*t;

  Vel.Rotation();
  SetRot( Vel.GetSafeNormal().Rotation() );

  // Flush the computed position to the root component
  RootComponent->SetWorldLocation( Pos );
}

void AProjectile::SetDestinationArc( FVector start, FVector end, float speed, float h )
{
  FVector dir = end - start;
  float len = dir.Size();
  Vel = dir;
  Vel.Z = 0.f;
  Vel.Normalize();
  Vel *= speed; // XY Velocity

  float travelTime = len / speed;
  float t = travelTime/2.f; // Time to reach full height.

  Vel.Z = 2*h/t;
  Gravity = -Vel.Z*Vel.Z / (2.f*h); // Acceleration for a velocity of 0 @ len/2.

  FVector simVel = Vel;
  FVector pos = start;
  for( float accumTime = 0; accumTime <= travelTime; accumTime += Game->gm->T )
  {
    pos += simVel*Game->gm->T; // 
    simVel.Z += Gravity * Game->gm->T;
  }
  
  SetPosition( start );
}
