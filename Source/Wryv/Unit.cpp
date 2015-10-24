#include "Wryv.h"

#include <map>
using namespace std;

#include "Item.h"
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
  for( int i = 0; i < Abilities.Num(); i++ )
  {
    if( Abilities[i] )
    {
      UUnitAction* action = Construct<UUnitAction>( Abilities[i] );
      action->Unit = this;
      CountersAbility.Push( action );
    }
  }

  for( int i = 0; i < StartingItems.Num(); i++ )
  {
    AddItem( StartingItems[i] );
  }

}

void AUnit::AddItem( UClass* ItemClass )
{
  if( !ItemClass )
  {
    error( FS( "%s::AddItem() ItemClass was null", *GetName() ) );
    return;
  }

  int i = IndexOfIsA( CountersItems, ItemClass );
  if( i >= 0 )
  {
    // Same class of item, increase qty
    if( CountersItems[i]->IsA( ItemClass ) )
    {
      CountersItems[i]->Quantity++;
      info( FS( "%s already had a %s, increased qty now have %d",
        *GetName(), *CountersItems[i]->GetName(), CountersItems[i]->Quantity ) );
      return;
    }
  }

  // 
  info( FS( "Constructing action %s", *ItemClass->GetName() ) );
  UItemAction* action = Construct<UItemAction>( ItemClass );
  action->AssociatedUnit = this;
  action->AssociatedUnitName = GetName();
  CountersItems.Push( action );
  
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
  if( index < 0 || index > CountersAbility.Num() )
  {
    error( FS( "%s cannot use ability %d, OOB", *Stats.Name, index ) );
    return 0;
  }

  UUnitAction* unitAction = CountersAbility[ index ];
  Game->hud->Status( FS( "%s %s", *Stats.Name, *unitAction->Text ) );
  info( FS( "Using Action %s [%s] on %s", *unitAction->Text,
    *GetEnumName( TEXT("Abilities"), unitAction->Ability ), *Stats.Name ) );
  Game->hud->SetNextAbility( unitAction->Ability.GetValue() );

  return 1;
}

bool AUnit::UseItem( int index )
{
  // Item instances already populate the CounterItems
  if( index < 0 || index >= CountersItems.Num() )
  {
    error( FS( "%s cannot consume item %d / %d, OOR",
      *Stats.Name, index, CountersItems.Num() ) );
    return 0;
  }

  info( FS( "%s is using item %s", *GetName(), *CountersItems[index]->GetName() ) );
  UItemAction* itemAction = CountersItems[index];
  BonusTraits.push_back( PowerUpTimeOut( Game->GetData( itemAction->ItemClass ) ) );

  // Use the item. qty goes down by 1
  // we don't affect the UI here, only
  itemAction->Quantity--;
  if( itemAction->Quantity <= 0 )
    CountersItems.RemoveAt( index );

  Game->hud->ui->dirty = 1;
  return 1;
}

void AUnit::MoveCounters( float t )
{
  AGameObject::MoveCounters( t );

  for( int i = 0; i < CountersAbility.Num(); i++ )
    CountersAbility[i]->Step( t );

  for( int i = 0; i < CountersItems.Num(); i++ )
    CountersItems[i]->Step( t );
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
      Face( AttackTarget->Pos );
      MoveWithinDistanceOf( AttackTarget, Stats.AttackRange * 0.9f );
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

