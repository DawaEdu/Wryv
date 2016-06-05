#include "Wryv.h"

#include <map>
using namespace std;

#include "Game/Enums.h"
#include "UE4/Flycam.h"
#include "GameObjects/Things/Items/Item.h"
#include "Util/ItemActionClassAndQuantity.h"
#include "UE4/PlayerControl.h"
#include "GameObjects/Things/Projectile.h"
#include "UE4/TheHUD.h"
#include "GameObjects/Units/Unit.h"
#include "UI/HotSpot/Widget.h"
#include "UE4/WryvGameInstance.h"
#include "UE4/WryvGameMode.h"

#include "UI/UICommand/Command/UIUnitActionCommand.h"
#include "UI/UICommand/Command/UIItemActionCommand.h"

// Sets default values
AUnit::AUnit( const FObjectInitializer& PCIP ) : AGameObject( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, TEXT( "Mesh" ) );
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
      UUIUnitActionCommand* action = Construct<UUIUnitActionCommand>( Abilities[i] );
      CountersAbility.Push( action );
    }
  }

  for( int i = 0; i < StartingItems.Num(); i++ )
  {
    AddItem( StartingItems[i] );
  }

}

void AUnit::AddItem( FItemActionClassAndQuantity itemAndQuantity )
{
  TSubclassOf< UUIItemActionCommand > itemActionClass = itemAndQuantity.ItemActionClass;
  if( !itemActionClass )
  {
    error( FS( "%s found item with ItemQuantity with null Action class", *GetName() ) );
    return;
  }

  int i = GetIndexWhichIsA( CountersItems, itemActionClass );
  if( i >= 0 )
  {
    // Same class of item, increase qty
    CountersItems[i]->Quantity++;
  }
  else
  {
    UUIItemActionCommand* itemAction = Construct<UUIItemActionCommand>( itemActionClass );
    itemAction->AssociatedUnit = this;
    itemAction->AssociatedUnitName = GetName();
    // max the cooldown
    itemAction->cooldown.TotalTime = itemAction->GetCooldownTotalTime();
    itemAction->cooldown.Finish();
    CountersItems.Push( itemAction );
  }

  // Since this gets called in beginplay(), we need to check the HUD is ready
  if( Game->hud )
    Game->hud->ui->dirty = 1;
}

void AUnit::PostInitializeComponents()
{
  Super::PostInitializeComponents();
}

void AUnit::BeginPlay()
{
  Super::BeginPlay();
}

void AUnit::UseAbility( int ability, AGameObject* target, FVector pos )
{
  switch( ability )
  {
    case Abilities::Movement:
      // Explicit movement, without attack possible.
      if( target == Game->flycam->floor )
        GoToGroundPosition( pos );
      else
        Follow( target );  // otherwise, follow clicked unit
      Game->hud->SkipNextMouseUp = 1;
      break;
    case Abilities::Attack:
      // Attack only, even friendly units.
      if( target == Game->flycam->floor )
        AttackGroundPosition( pos );  // ready to attack enemy units
      else
        Attack( target );
      Game->hud->SkipNextMouseUp = 1;
      break;
    case Abilities::Stop:
      // Stops units from moving
      Stop();
      break;
    case Abilities::HoldGround:
      HoldGround();
      break;
    default:
      error( "Ability NotSet" );
      break;
  }
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
  UUIItemActionCommand* itemAction = CountersItems[index];
  BonusTraits.push_back( PowerUpTimeOut( 
    Game->GetData( itemAction->ItemClass ) ) );

  // Use the item. qty goes down by 1
  // we don't affect the UI here, only
  itemAction->Quantity--;
  if( itemAction->Quantity <= 0 )
    CountersItems.RemoveAt( index );
  else
  {
    // Reset the item counter
    itemAction->cooldown.Reset();
  }

  Game->hud->ui->dirty = 1;
  return 1;
}

void AUnit::MoveCounters( float t )
{
  AGameObject::MoveCounters( t );
  for( int i = (int)CountersAbility.Num() - 1; i >= 0; i-- )
    CountersAbility[i]->Step( t );
  for( int i = (int)CountersItems.Num() - 1; i >= 0; i-- )
    CountersItems[i]->Step( t );
}

void AUnit::OnUnselected()
{
  AGameObject::OnUnselected();
  for( int i = 0; i < CountersAbility.Num(); i++ )
    CountersAbility[i]->clock = 0;
  for( int i = 0; i < CountersItems.Num(); i++ )
    CountersItems[i]->clock = 0;
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
  FVector launchPos = Pos;
  
  // Try and get the launch socket of the mesh. if it doesn't have one, use the top of the object.
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>();
  // Check all the meshes for a mesh with socket named "Weapon"
  for( UMeshComponent* mesh : meshes )
  {
    if( mesh->DoesSocketExist( "Weapon" ) )
    {
      launchPos = mesh->GetSocketLocation( "Weapon" );
      //info( FS( "Launch socket @ %f %f %f", launchPos.X, launchPos.Y, launchPos.Z ) );
    }
  }
  //launchPos.Z += 25.f;
  //Game->flycam->DrawDebug( launchPos, 10.f, FLinearColor::Blue, 10.f );
  //LOG( "%s launching a projectile of type %s", *Stats.Name, *ReleasedProjectileWeapon->GetName() );
  AProjectile* projectile = Game->Make<AProjectile>( ReleasedProjectileWeapon, team, launchPos );
  if( !projectile )
  {
    error( FS( "Projectile couldn't be launched from %s", *Stats.Name ) );
  }
  else
  {
    projectile->Shooter = this;
    projectile->Target( AttackTarget ); // Target my current attack target
    projectile->SetDestinationArc( launchPos, AttackTarget->Pos );
  }
}


