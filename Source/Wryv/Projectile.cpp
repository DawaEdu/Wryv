#include "Wryv.h"
#include "Projectile.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"
#include "FlyCam.h"

AProjectile::AProjectile(const FObjectInitializer& PCIP) : AGameObject(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
  Shooter = 0;
  GroundTraveller = 0;
  Gravity = 50.f;
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
  //////AGameObject::Move( t ); // OVERRIDES BASE
  Vel.Z += -Gravity * t;
  Pos += Vel * t;

  Vel.Rotation();
  SetRot( Vel.GetSafeNormal().Rotation() );

  // Flush the computed position to the root component
  RootComponent->SetWorldLocation( Pos );

  //LOG( "@ Vel=%f %f %f", Vel.X, Vel.Y, Vel.Z );
}

void AProjectile::SetDestinationArc( FVector start, FVector end )
{
  FVector dir = end - start;
  float len = dir.Size();
  Vel = dir;
  Vel.Z = 0.f;
  Vel.Normalize();
  Vel *= BaseStats.SpeedMax; // XY Velocity

  // Time to reach: horizontal speed
  float t = len / BaseStats.SpeedMax;
  // Initial vertical velocity is solution of d = v1t + .5*t*t, where v1 = d/t - .5*t
  float h = BaseStats.MaxTravelHeight;
  float v1 = (h - .5f*(-Gravity)*t*t)/t;

  //float v2 = sqrtf( 2 * Gravity * h );
  //LOG( "1: %f 2: %f", v1, v2 ); // V1 & V2 are very similar.
  Vel.Z = v1;
  Game->flycam->ClearViz();
  Game->flycam->Visualize( Types::UNITSPHERE, start, 50.f, FLinearColor::White );
  Game->flycam->Visualize( Types::UNITSPHERE, end, 50.f, FLinearColor::Black );
  FVector d = start;
  for( float i = 0; i <= t; i += t/15.f )
  {
    FVector pt = start + Vel*t;
    LOG( "Point %f %f %f", pt.X, pt.Y, pt.Z );
    Game->flycam->Visualize( Types::UNITSPHERE, pt, 45.f, FLinearColor::Red );
  }

  SetPosition( start );
}
