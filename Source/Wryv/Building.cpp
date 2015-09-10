#include "Wryv.h"
#include "Building.h"
#include "GlobalFunctions.h"
#include "WryvGameInstance.h"
#include "FlyCam.h"
#include "Explosion.h"
#include "Peasant.h"
#include "PlayerControl.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Runtime/Engine/Classes/Particles/ParticleEmitter.h"

ABuilding::ABuilding( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
  peasant = 0;
  Complete = 0;
  TimeBuilding = FLT_MAX; // So the building starts complete.
  ExplodedTime = 0.f;
  
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, "buildingmesh" );
  Mesh->AttachTo( DummyRoot );
  ExitPosition = PCIP.CreateDefaultSubobject<USceneComponent>( this, "ExitPosition" );
  ExitPosition->AttachTo( DummyRoot );
  destructableMesh = PCIP.CreateDefaultSubobject<UDestructibleComponent>( this, "DestructibleMesh" );
  destructableMesh->AttachTo( DummyRoot );
  buildingDust = PCIP.CreateDefaultSubobject<UParticleSystemComponent>( this, "ParticleEmitter" );
  buildingDust->AttachTo( DummyRoot );
}

void ABuilding::PostInitializeComponents()
{
  Super::PostInitializeComponents();

  // start the destructible mesh as invisible
  Mesh->SetVisibility( true );
  destructableMesh->SetVisibility( false );
}

void ABuilding::BeginPlay()
{
  Super::BeginPlay();
  TimeBuilding = Stats.TimeLength; // Start as completed (for ghost object placement).
  Hp = Stats.HpMax;//  Start with 1 hp so that the building doesn't bust immediately
}

void ABuilding::Move( float t )
{
  AGameObject::Move( t ); // Updates Dead variable

  if( Dead )
  {
    if( !ExplodedTime )  BuildingExploded();
    ExplodedTime += t;
  }

  // Peasant's required to continue the building.
  if( peasant && TimeBuilding < Stats.TimeLength )
  {
    TimeBuilding += t;
    UAnimInstance* anim = Mesh->GetAnimInstance();
    if( anim )
    {
      FAnimMontageInstance* fmontage = anim->GetActiveMontageInstance();
      if( fmontage )
      {
        int frames = fmontage->Montage->GetNumberOfFrames();
        float len = fmontage->Montage->GetTimeAtFrame( frames );
        //info( FS( "fmontage is SET, frames=%d, len=%f", frames, len ) );
        float p = len * PercentBuilt();
        fmontage->SetPosition( p );
      }
      else
      {
        info( FS( "there is No fmontage" ) );
      }
    }

    // keep the dust on while the building is being built.
    buildingDust->SetActive( true );

    // Building not complete yet, so increase HP and increase HP by a fraction
    Hp += t / Stats.TimeLength * Stats.HpMax;// hp increases because building may be attacked while being built.

    // The building cannot do anything else while building.
    Clamp( Hp, 0.f, Stats.HpMax );
  }
  else if( Complete )
  {
    buildingDust->SetActive( false );
    //buildingDust->SetVisibility( false );
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

void ABuilding::montageStarted_Implementation( UAnimMontage* Montage )
{
  LOG( "PLAYING MONTAGE %d", Montage );
}

bool ABuilding::CanBePlaced()
{
  set<AGameObject*> objs;
  if( peasant )
  {
    objs = Game->pc->PickExcept( this, {peasant} );
  }
  else
  {
    objs = Game->pc->Pick( this );
  }
  for( AGameObject* o : objs )
  {
    LOG( "building %s intersects with %s", *Stats.Name, *o->Stats.Name );
  }

  return !objs.size();
}

void ABuilding::PlaceBuilding( APeasant *p )
{
  SetMaterialColors( "Multiplier", FLinearColor( 1,1,1,1 ) );
  TimeBuilding = 0; // Reset the build counter.
  Hp = 1.f;
  // move the selected peasant to work with the building.
  //PlaySound( UISounds::BuildingPlaced );
  // Selected objects will go build it send the peasant to build the building
  if( !p )
  {
    warning( FS( "Building %s is building itself", *Stats.Name ) );
    return;
  }

  // start the building MONTAGE
  UAnimInstance* anim = Mesh->GetAnimInstance();
  anim->Montage_Play( buildingMontage, 0.01f );

  // Move the peasant inside the building, then hide.
  // The peasant must be allowed inside the hitbounds.
  p->hitBounds->SetCollisionEnabled( ECollisionEnabled::NoCollision ); // deactivate collisions?
  p->SetDestination( Pos );
  p->OnReachDestination = [this,p]()
  {
    // put the peasant underground, so it appears to be building.
    peasant = p; // only set the peasant here, so building only starts once he gets there.
    peasant->SetPosition( peasant->Pos - FVector( 0, 0, peasant->GetHeight() ) );
    peasant->hitBounds->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
  };
}

void ABuilding::OnBuildingComplete()
{
  LOG( "Job's done" );
  if( peasant )
  {
    peasant->SetPosition( ExitPosition->GetComponentLocation() );
  }

  Complete = 1;
}

void ABuilding::BuildingExploded_Implementation()
{
  MakeChild<AExplosion>( Stats.OnContact );
  Mesh->SetVisibility( false ); // hide the normal mesh
  destructableMesh->SetVisibility( true ); // use destructible mesh
  destructableMesh->ApplyRadiusDamage( 111, Pos, 10.f, 50000.f, 1 );
}

void ABuilding::Die()
{
  AGameObject::Die();
}


