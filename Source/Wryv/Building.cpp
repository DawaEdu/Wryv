#include "Wryv.h"
#include "Building.h"
#include "GlobalFunctions.h"
#include "WryvGameInstance.h"
#include "Explosion.h"
#include "Peasant.h"
#include "Runtime/Engine/Classes/Particles/ParticleEmitter.h"

ABuilding::ABuilding( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
  peasant = 0;
  CanBePlaced = 1;
  Complete = 0;
  TimeBuilding = FLT_MAX; // So the building starts complete.
  ExplodedTime = 0.f;
  
  ExitPosition = PCIP.CreateDefaultSubobject<USceneComponent>( this, "ExitPosition" );
  ExitPosition->AttachTo( DummyRoot );
  destructableMesh = PCIP.CreateDefaultSubobject<UDestructibleComponent>( this, "DestructibleMesh" );
  destructableMesh->AttachTo( DummyRoot );
  buildingDust = PCIP.CreateDefaultSubobject<UParticleSystemComponent>( this, "ParticleEmitter" );
  buildingDust->AttachTo( DummyRoot );
}

void ABuilding::BeginPlay()
{
  Super::BeginPlay();
  TimeBuilding = Stats.TimeLength; // Start as completed (for ghost object placement).
}

void ABuilding::Move( float t )
{
  AGameObject::Move( t ); // Updates Dead variable

  if( Dead )
  {
    if( !ExplodedTime )  BuildingExploded();
    ExplodedTime += t;
  }

  TimeBuilding += t;
  if( TimeBuilding < Stats.TimeLength )
  {
    // Building not complete yet, so increase HP and increase HP by a fraction
    Hp += t / Stats.TimeLength * Stats.HpMax;// hp increases because building may be attacked while being built.
    // The building cannot do anything else while building.
    Clamp( Hp, 0.f, Stats.HpMax );
  }
  else if( !Complete )
  {
    // Building complete. Send building peasant outside of building & notify
    OnBuildingComplete();
  }
}

// After a building gets destroyed, it is out of play. The post-die() destruction code
// runs in ::Tick(), since it is not important to gamestate.
void ABuilding::Tick( float t )
{
  Super::Tick( t );

  if( Dead )
  {
    ExplodedTime += t;

    if( ExplodedTime > MaxExplosionTime )
    {
      Destroy();
      return;
    }
  }
}

void ABuilding::SetPeasant( APeasant *p )
{
  peasant = p;
  // Move the peasant inside the building, then hide.
  peasant->SetDestination( Pos );
  peasant->OnReachDestination = [this,p]()
  {
    // put the peasant underground, so it appears to be building.
    p->SetPosition( p->Pos - FVector( 0, 0, p->GetHeight() ) );
  };
}

void ABuilding::OnBuildingComplete()
{
  LOG( "Job's done" );
  if( peasant )
  {
    peasant->SetPosition( ExitPosition->GetComponentLocation() );
  }

  buildingDust->SetActive( false );
  Complete = 1;
}

void ABuilding::BuildingExploded_Implementation()
{
  MakeChild<AExplosion>( Stats.OnContact );
  destructableMesh->SetVisibility( true );
  destructableMesh->ApplyRadiusDamage( 111, Pos, 10.f, 50000.f, 1 );
}

void ABuilding::Die()
{
  AGameObject::Die();
}


