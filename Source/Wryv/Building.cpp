#include "Wryv.h"

#include "Building.h"
#include "Explosion.h"
#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "Peasant.h"
#include "PlayerControl.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

#include "Animation/AnimNode_StateMachine.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Runtime/Engine/Classes/Particles/ParticleEmitter.h"

#include "InProgressUnit.h"
#include "InProgressResearch.h"
#include "Research.h"
#include "TrainingAction.h"

ABuilding::ABuilding( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
  Complete = 0;
  TimeBuilding = FLT_MAX; // So the building starts complete.
  ExplodedTime = 0.f;
  MaxDeadTime = MaxExplosionTime;
  ExplosiveRadius = 11.f;
  ExplosiveForce = 50000.f;
  
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

  Recovering = 0; // buildings don't autorecover hp.
}

void ABuilding::BeginPlay()
{
  LOG( "ABuilding::BeginPlay()");
  Super::BeginPlay();
  TimeBuilding = Stats.TimeLength; // Start as completed (for ghost object placement).
  Hp = Stats.HpMax; // Start with 1 hp so that the building doesn't bust immediately
}

void ABuilding::InitIcons()
{
  AGameObject::InitIcons();
  
  TrainingAvailable.Empty();
  for( int i = 0; i < TrainClasses.Num(); i++ )
  {
    if( TrainClasses[i] )
    {
      UTrainingAction* action = Construct<UTrainingAction>( TrainClasses[i] );
      action->Building = this;
      TrainingAvailable.Push( action );
    }
  }

  ResearchesAvailable.Empty();
  for( int i = 0; i < ResearchClasses.Num(); i++ )
  {
    if( ResearchClasses[i] )
    {
      UResearch* action = Construct<UResearch>( ResearchClasses[i] );
      action->Building = this;
      ResearchesAvailable.Push( action );
    }
  }
}

void ABuilding::LosePeasant( APeasant* peasant )
{
  //Game->hud->Status( FS( "Building %s lost PrimaryPeasant builder %s", *Name, *peasant->Name ) );
  // The building loses its peasant builder.
  if( peasant == PrimaryPeasant )
  {
    // Need to try to replace primary peasant from among other builders
    PrimaryPeasant = 0;
  
    // Take the first available peasant follower/builder and make it the primary
    for( int i = 0; i < Followers.size(); i++ )
    {
      if( APeasant* p = Cast<APeasant>( Followers[i] ) )
      {
        PrimaryPeasant = p;
        break;
      }
    }
  }
}

void ABuilding::MoveCounters( float t )
{
  AGameObject::MoveCounters( t );
  // When moving counters, when a counter expires it gets removed, 
  for( int i = (int)TrainingAvailable.Num() - 1; i >= 0; i-- )
    TrainingAvailable[i]->Step( t );
  for( int i = (int)CountersUnitsInProgress.Num() - 1; i >= 0; i-- )
    CountersUnitsInProgress[i]->Step( t );
  for( int i = (int)ResearchesAvailable.Num() - 1; i >= 0; i-- )
    ResearchesAvailable[i]->Step( t );
  for( int i = (int)CountersResearchInProgress.Num() - 1; i >= 0; i-- )
    CountersResearchInProgress[i]->Step( t );
}

void ABuilding::Move( float t )
{
  AGameObject::Move( t ); // Updates Dead variable
  // Peasant's required to continue the building.

  if( TimeBuilding < Stats.TimeLength )
  {
    // If the Primary Peasant is within building distance, contribute to building.
    // Other units repair the building.
    if( PrimaryPeasant && PrimaryPeasant->isFollowTargetWithinRange() )
    {
      TimeBuilding += t;
      if( UAnimInstance* anim = Mesh->GetAnimInstance() )
      {
        FAnimMontageInstance* fmontage = anim->GetActiveMontageInstance();
        if( fmontage )
        {
          float len = fmontage->Montage->GetPlayLength();
          float p = len * PercentBuilt();
          fmontage->SetPosition( p );
        }
        else
        {
          error( FS( "there is No fmontage" ) );
        }
      }
      // keep the dust on while the building is being built.
      buildingDust->SetActive( true );
      // Building not complete yet, so increase HP and increase HP by a fraction
      Hp += GetHPAdd( t );// hp increases because building may be attacked while being built.
      // The building cannot do anything else while building.
      Clamp( Hp, 0.f, Stats.HpMax );
    }

    if( !PrimaryPeasant )
    {
      // Visually indicate the building has no primary peasant builder
    }
  }
  else if( !Complete )// Building time up, but not marked as complete yet
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
    // This has to be here because
    // ::Move() won't get called for dead objects (since they get removed
    // from the Team when they die)
    ExplodedTime += t;
    if( ExplodedTime > MaxExplosionTime )
      Cleanup();
  }
}

bool ABuilding::UseTrain( int index )
{
  if( index < 0 || index >= TrainingAvailable.Num() )
  {
    error( FS( "index %d OOB", index ) );
    return 0;
  }

  // Construct an in-progress counter & add it to counters for this object
  UInProgressUnit* unitInProgress = Construct<UInProgressUnit>( UInProgressUnit::StaticClass() );
  unitInProgress->Set( TrainingAvailable[ index ] );
  CountersUnitsInProgress.Push( unitInProgress );
   
  // Change to UI:
  Game->hud->ui->dirty = 1; // Refresh with selected units
  
  return 1;
}

bool ABuilding::CancelTraining( int index )
{
  if( index < 0 || index >= CountersUnitsInProgress.Num() )
  {
    error( FS( "CancelTraining index %d OOB", index ) );
    return 0;
  }

  UInProgressUnit* ipUnit = CountersUnitsInProgress[ index ];

  // refund
  team->Refund( ipUnit->UnitType );
  // remove elt
  CountersUnitsInProgress.RemoveAt( index );

  return 1;
}

bool ABuilding::UseResearch( int index )
{
  if( index < 0 || index >= ResearchesAvailable.Num() )
  {
    error( FS( "index %d OOB", index ) );
    return 0;
  }

  // Start the clock on there
  UResearch* research = ResearchesAvailable[ index ];
  
  // Spend money required to upgrade
  team->Gold   -= research->GoldCost;
  team->Lumber -= research->LumberCost;
  team->Stone  -= research->StoneCost;
  
  UInProgressResearch* iPResearch = Construct<UInProgressResearch>( UInProgressResearch::StaticClass() );
  iPResearch->Set( research );
  iPResearch->UActionIndex = index;
  CountersResearchInProgress.Push( iPResearch );
  
  // Change to UI:
  Game->hud->ui->dirty = 1; // Refresh with selected units
  
  return 1;
}

bool ABuilding::CancelResearch( int index )
{
  // Cancels ith research
  if( index < 0 || index >= CountersResearchInProgress.Num() )
  {
    error( FS( "CancelResearch index %d OOB", index ) );
    return 0;
  }

  // Removing the elt cancels the research.
  CountersResearchInProgress.RemoveAt( index );

  return 1;
}

void ABuilding::OnUnselected()
{
  AGameObject::OnUnselected();
  for( int i = 0; i < CountersUnitsInProgress.Num(); i++ )
    CountersUnitsInProgress[i]->clock = 0;
  for( int i = 0; i < CountersResearchInProgress.Num(); i++ )
    CountersResearchInProgress[i]->clock = 0;
}

void ABuilding::montageStarted_Implementation( UAnimMontage* Montage )
{
  LOG( "PLAYING MONTAGE %d", Montage );
}

bool ABuilding::CanBePlaced()
{
  // Required AGameObject* for collision check, not APeasant*
  vector<AGameObject*> objs = Game->pc->ComponentPickExcept(
    this, hitBounds, { PrimaryPeasant }, "Checkers", {"SolidObstacle"} );
  for( AGameObject* go : objs )
  {
    LOG( "Building %s hits object %s", *GetName(), *go->GetName() );
  }
  return !objs.size();
}

void ABuilding::PlaceBuilding( APeasant *p )
{
  p->Target( this );

  SetMaterialColors( "Multiplier", FLinearColor( 1,1,1,1 ) );
  TimeBuilding = 0; // Reset the build counter.
  Hp = 1.f;
  // move the selected peasant to work with the building.
  //PlaySound( UISounds::BuildingPlaced );
  // Selected objects will go build it send the peasant to build the building
  if( !PrimaryPeasant )
  {
    warning( FS( "Building %s is building itself", *Stats.Name ) );
    return;
  }

  // Start the building MONTAGE
  UAnimInstance* anim = Mesh->GetAnimInstance();
  anim->Montage_Play( buildingMontage, 0.001f );

  // Target the building to "repair it" until it is complete.
  if( PrimaryPeasant )
    Game->EnqueueCommand( Command( Command::Target, PrimaryPeasant->ID, ID ) );
}

void ABuilding::ReleaseUnit( UInProgressUnit* unit )
{
  Game->Make< AUnit >( unit->UnitType, team, GetExitPosition() );
  CountersUnitsInProgress.Remove( unit );
}

void ABuilding::DropBuilders( bool buildingSuccess )
{
  if( PrimaryPeasant )
  {
    if( buildingSuccess )
      PrimaryPeasant->JobDone();
    PrimaryPeasant->SetPosition( GetExitPosition() );
  }

  // All builders stop building this building.
  for( AGameObject *go : Followers )
  {
    // All peasants who were repair/building this building stop repairing it.
    if( APeasant * peasant = Cast<APeasant>( go ) )
      if( peasant->RepairTarget == this )
        go->Follow(0);
  }
}

void ABuilding::OnBuildingComplete()
{
  DropBuilders(1);
  // Turn off the dust
  buildingDust->SetActive( false );
  Complete = 1;
}

void ABuilding::Cancel()
{
  DropBuilders(0);
  Die();
}

void ABuilding::Die()
{
  Mesh->SetVisibility( false ); // hide the normal mesh
  destructableMesh->SetVisibility( true ); // use destructible mesh
  destructableMesh->ApplyRadiusDamage( 111, Pos, 10.f, 50000.f, 1 ); // Shatter the destructable.

  // If the building peasant was still there, let him discontinue building the building
  DropBuilders(0);

  AGameObject::Die();
}


