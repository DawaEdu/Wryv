#include "Wryv.h"

#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "Projectile.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

AProjectile::AProjectile( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
  Shooter = 0;
  Gravity = -10.f;
  MaxTravelHeight = 150.f;
  Homing = 0;
}

void AProjectile::ai( float t )
{
  // The AI for the spell is basically nothing at this point,
  // though we might want to add spin/curl to some spells
}

void AProjectile::Move( float t )
{
  LifeTime += t;
  if( LifeTime >= MaxLifeTime )
  {
    // Kill spells that don't arc after they exceed duration.
    Die(); Cleanup();
  }

  if( !Homing )
  {
    // If it is a homing attack, then flight is different.
    Vel.Z += Gravity * t;
    Pos += Vel*t;
    SetRot( Vel.GetSafeNormal().Rotation() );
    FlushPosition();
  }
  else
  {
    MoveCounters( t );
    if( CheckDead( t ) )  return;
    CheckNextCommand();
    UpdateStats( t );
    //RecomputePath(); // Recomputes path towards destination, if applicable
    Walk( t );   // Walk towards destination
    FlushPosition();
  }
}

void AProjectile::Hit( AGameObject* other )
{
  if( !other ) {
    error( FS( "%s Hit NULL object", *Stats.Name ) );
    return;
  }

  if( other == Shooter ) {
    return;
  }

  // If the projectile is running into its attack target,
  // OR the object has hit the floor, then allow it to deal damage
  if( other == AttackTarget   &&   !AttackTarget->Dead )
  {
    info( FS( "%s is detonating", *GetName() ) );
    // Damage the attack target with impact-damage
    AttackTarget->ReceiveAttack( this );
    Die(); Cleanup();
  }
  else if( other == Game->flycam->floor )
  {
    info( FS( "%s is contacting the floor", *Stats.Name ) );
    Die(); Cleanup();
  }
}

void AProjectile::RecomputePath()
{
  if( Homing )
  {
    AGameObject::RecomputePath(); // Recompute the pathway to the 
  }
}

void AProjectile::SetDestinationArc( FVector start, FVector end )
{
  float h = MaxTravelHeight;
  Dir = end - start;
  float len = Dir.Size();
  Vel = Dir;
  Vel.Z = 0.f;
  Vel.Normalize();
  Vel *= Stats.SpeedMax; // XY Velocity

  float travelTime = len / Stats.SpeedMax;
  float t = travelTime/2.f; // Time to reach full height.
  // Simultaneously solve:
  //   v_2^2 = v_1^2 + 2*a*len
  //   h = v_1*t + .5*a*t*t
  Vel.Z = 2.f*h/t;
  Gravity = -Vel.Z*Vel.Z / (2.f*h); // Acceleration for a velocity of 0 @ len/2.

  SetPosition( start );
}

void AProjectile::Die()
{
  AGameObject::Die();
  if( OnImpact )
  {
    Game->Make<AExplosion>( OnImpact, team, Pos );
  }
}
