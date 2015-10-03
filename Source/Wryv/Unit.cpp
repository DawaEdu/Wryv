#include "Wryv.h"

#include <map>
using namespace std;

#include "PlayerControl.h"
#include "TheHUD.h"
#include "Unit.h"
#include "Widget.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

#include "UnitAction.h"
#include "ItemAction.h"

// Sets default values
AUnit::AUnit( const FObjectInitializer& PCIP ) : AGameObject( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, "themeshof" );
  Mesh->AttachTo( DummyRoot );
}

void AUnit::InitIcons()
{
  AGameObject::InitIcons();

  // Create instances of each class type
  CountersAbility.clear();
  for( int i = 0; i < Abilities.Num(); i++ )
  {
    if( Abilities[i] )
    {
      UUnitAction* action = NewObject<UUnitAction>( this, Abilities[i] );
      action->Unit = this;
      info( FS( "Ability %s has been added %s", *action->GetName(), *action->Unit->GetName() ) );
      CountersAbility.push_back( action );
    }
  }

  CountersItems.clear();
  for( int i = 0; i < StartingItems.Num(); i++ )
  {
    if( StartingItems[i] )
    {
      UItemAction* action = NewObject<UItemAction>( this, StartingItems[i] );
      action->Unit = this;
      info( FS( "Ability %s has been added %s", *action->GetName(), *action->Unit->GetName() ) );
      CountersItems.push_back( action );
    }
  }
}

void AUnit::PostInitializeComponents()
{
  Super::PostInitializeComponents();
}

void AUnit::BeginPlay()
{
  Super::BeginPlay();
}

bool AUnit::UseAbility( int index )
{
  if( index < 0 || index > CountersAbility.size() )
  {
    error( FS( "%s cannot use ability %d, OOB",
      *Stats.Name, index ) );
    return 0;
  }

  if( !CountersAbility[index]->IsReady() )
  {
    info( FS( "%s: Ability %s was not ready",
      *Stats.Name, *CountersAbility[index]->Text ) );
    return 0;
  }
  
  if( AUnit* unit = Cast<AUnit>(this) )
    CountersAbility[index]->Click();
  else
    error( "not a unit" );
  return 1;
}

bool AUnit::UseItem( int index )
{
  // Item instances already populate the CounterItems
  if( index < 0 || index >= CountersItems.size() )
  {
    error( FS( "%s cannot consume item %d / %d, OOR",
      *Stats.Name, index, CountersItems.size() ) );
    return 0;
  }

  // use the item. qty goes down by 1
  // we don't affect the UI here, only
  CountersItems[index]->Quantity--;
  if( !CountersItems[index]->Quantity ) {
    removeIndex( CountersItems, index );
  }
  return 1;
}

void AUnit::Move( float t )
{
  // recompute path
  if( Stats.SpeedMax )
  {
    // Prioritize the FollowTarget.
    if( FollowTarget && AttackTarget )
    {
      warning( FS( "%s had both FollowTarget=%s AttackTarget=%s",
        *Stats.Name, *FollowTarget->Stats.Name, *AttackTarget->Stats.Name ) );
    }

    if( FollowTarget )
    {
      MoveWithinDistanceOf( FollowTarget,  FollowTarget->Radius() );
    }
    else if( AttackTarget )
    {
      MoveWithinDistanceOf( AttackTarget, Stats.AttackRange * 0.9f );
      Face( AttackTarget->Pos );
    }
  }

  Walk( t );   // Walk towards destination
  AGameObject::Move( t );
}

void AUnit::AttackCycle()
{
  if( !AttackTarget )
  {
    LOG( "The target is missing, attack cycle should not have been called." );
    return;
  }

  if( AttackTarget )
  {
    if( AttackTarget->Dead )
    {
      error( "Attacking a dead target" );
      return;
    }

    if( ReleasedProjectileWeapon )
    {
      Shoot();
    }
    else
    {
      // Sword attack
      AttackTarget->ReceiveAttack( this );
    }
  }
}

void AUnit::Shoot()
{
  //info( FS( "%s is shooting a %s to %s", *Name, *GetTypesName(ReleasedProjectileWeapon),
  //  *AttackTarget->Name ) );
  FVector launchPos = GetTip();
  
  // Try and get the launch socket of the mesh. if it doesn't have one, use the top of the object.
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>();
  // Check all the meshes for a mesh with socket named "Weapon"
  for( UMeshComponent* mesh : meshes )
  {
    if( mesh->DoesSocketExist( "Weapon" ) )
    {
      launchPos = mesh->GetSocketLocation( "Weapon" );
      info( FS( "Launch socket @ %f %f %f", launchPos.X, launchPos.Y, launchPos.Z ) );
    }
  }

  LOG( "%s launching a projectile of type %s", *Stats.Name, *ReleasedProjectileWeapon->GetName() );
  AProjectile* projectile = Game->Make<AProjectile>( ReleasedProjectileWeapon, team, launchPos );
  if( !projectile )
  {
    error( FS( "Projectile couldn't be launched from %s", *Stats.Name ) );
  }
  else
  {
    projectile->Shooter = this;
    projectile->Attack( AttackTarget );
    projectile->AttackTargetOffset = AttackTargetOffset;
    
    projectile->SetDestinationArc( launchPos, AttackTarget->GetCentroid(),
      projectile->Stats.SpeedMax, projectile->MaxTravelHeight );
  }
}

