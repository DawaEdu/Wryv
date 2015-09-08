#include "Wryv.h"
#include "GlobalFunctions.h"
#include "GameObject.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "FlyCam.h"
#include "Projectile.h"
#include "Pathfinder.h"
#include "GlobalFunctions.h"
#include "PlayerControl.h"
#include "Widget3D.h"
#include "Building.h"
#include "Explosion.h"


const float AGameObject::WaypointAngleTolerance = 30.f; // 
const float AGameObject::WaypointReachedToleranceDistance = 250.f; // The distance to consider waypoint as "reached"

AGameObject* AGameObject::Nothing = 0;

// Sets default values
AGameObject::AGameObject( const FObjectInitializer& PCIP )
{
  //LOG( "%s [%s]->AGameObject::AGameObject()", *GetName(), *BaseStats.Name );
  PrimaryActorTick.bCanEverTick = true;
  Repairing = 0;
  team = 0;
  Pos = Vel = FVector(0, 0, 0);
  FollowTarget = AttackTarget = 0;
  RepelMultiplier = 1.f;
  Dead = 0;

  DummyRoot = PCIP.CreateDefaultSubobject<USceneComponent>( this, "Dummy" );
  SetRootComponent( DummyRoot );
  hitBounds = PCIP.CreateDefaultSubobject<UCapsuleComponent>( this, "HitVolumex222" );
  hitBounds->AttachTo( DummyRoot );
  repulsionBounds = PCIP.CreateDefaultSubobject<USphereComponent>( this, "RepulsionVolumex22" );
  repulsionBounds->AttachTo( DummyRoot );
  OnReachDestination = function< void () >();
}

void AGameObject::PostInitializeComponents()
{
  //LOG( "%s [%s]->AGameObject::PostInitializeComponents()", *GetName(), *BaseStats.Name );
  Super::PostInitializeComponents();

  if( RootComponent )
  {
    // Initialize position, but put object on the ground
    SetPosition( RootComponent->GetComponentLocation() );

    // Attach contact function to all bounding components.
    hitBounds->OnComponentBeginOverlap.AddDynamic( this, &AGameObject::OnHitContactBegin );
    hitBounds->OnComponentEndOverlap.AddDynamic( this, &AGameObject::OnHitContactEnd );

    repulsionBounds->OnComponentBeginOverlap.AddDynamic( this, &AGameObject::OnRepulsionContactBegin );
    repulsionBounds->OnComponentEndOverlap.AddDynamic( this, &AGameObject::OnRepulsionContactEnd );
  }

  UpdateStats( 0.f );
  Hp = Stats.HpMax;
  Speed = 0.f;

  if( isBuilding() )  Repairing = 0; // Buildings need an attending peasant to repair
  else  Repairing = 1; // Live units automatically regen
}

// Called when the game starts or when spawned
void AGameObject::BeginPlay()
{
  Super::BeginPlay();

  //LOG( "%s [%s]->AGameObject::BeginPlay()", *GetName(), *BaseStats.Name );
  Team* newTeam = Game->gm->teams[ BaseStats.TeamId ];
  SetTeam( newTeam );

  // Instantiate abilities
  for( int i = 0; i < Stats.Abilities.Num(); i++ )
    AbilityCooldowns.push_back( CooldownCounter( Stats.Abilities[i] ) );
}

void AGameObject::OnMapLoaded()
{
  
}

AGameObject* AGameObject::SetParent( AGameObject* newParent )
{
  USceneComponent *pc = 0;
  if( newParent )  pc = newParent->GetRootComponent();
  SetOwner( newParent );

  // Keep original scale & reset
  FVector s = FVector( hitBounds->GetScaledCapsuleRadius() * 1.5f );
  //LOG( "Scaling %f %f %f", s.X, s.Y, s.Z );
  // Set the world position to being that of parent, then keep world position on attachment
  GetRootComponent()->SetWorldScale3D( FVector(1,1,1) ); // reset the scale off
  GetRootComponent()->SetRelativeScale3D( FVector(1,1,1) ); 
  GetRootComponent()->SnapTo( pc );

  GetRootComponent()->SetWorldScale3D( s ); // don't set scaling relative to parent.. set it absolute in world size coords
  return this;
}

AGameObject* AGameObject::AddChild( AGameObject* newChild )
{
  // To add another child to this actor, we need to attach the childs root component to this actor?
  newChild->SetParent( this );
  return this;
}

bool AGameObject::isParentOf( AGameObject* child )
{
  return child->GetOwner() == this;
}

bool AGameObject::isChildOf( AGameObject* parent )
{
  return IsAttachedTo( parent );
}

void AGameObject::SetSize( FVector size )
{
  float maxDimen = GetComponentsBoundingBox().GetExtent().GetMax();
  if( !maxDimen )
  {
    LOG( "::SetSize(): %s had %f size bounding box", *GetName(), maxDimen );
    SetActorScale3D( size );
  }
  else
    SetActorScale3D( size/maxDimen );
}

FVector AGameObject::GetTip()
{
  return Pos + FVector( 0,0, hitBounds->GetScaledCapsuleHalfHeight()*2.f );
}

FVector AGameObject::GetCentroid()
{
  return hitBounds->GetComponentLocation();
}

float AGameObject::GetHeight()
{
  return hitBounds->GetScaledCapsuleHalfHeight() * 2.f;
}

float AGameObject::centroidDistance( AGameObject *go )
{
  if( !go ) {
    LOG( "centroidDistance( 0 ): null" );
    return FLT_MAX;
  }
  return FVector::Dist( Pos, go->Pos );
}

float AGameObject::outsideDistance( AGameObject *go )
{
  if( !go ) {
    LOG( "outsideDistance( 0 ): null" );
    return FLT_MAX;
  }
  float r1 = GetBoundingRadius();
  float r2 = go->GetBoundingRadius();
  return centroidDistance( go ) - (r1 + r2);
}

bool AGameObject::isAttackTargetWithinRange()
{
  if( AttackTarget )
  {
    return centroidDistance( AttackTarget ) < Stats.AttackRange;
  }

  return 0;
}

float AGameObject::HpPercent()
{
  if( ! Stats.HpMax ) {
    error( FS( "HpMax not set for %s", *Stats.Name ) );
    return 1.f;
  }
  else  return Hp / Stats.HpMax;  // if max hp not set, just return hp it has
}

float AGameObject::SpeedPercent()
{
  if( ! Stats.SpeedMax )
  {
    error( FS( "SpeedMax not set for %s", *Stats.Name ) );
    return 100.f;
  }
  return 100.f * Speed / Stats.SpeedMax;
}

void AGameObject::AttackCycle()
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

    if( Stats.ReleasedProjectileWeapon )
    {
      Shoot();
    }
    else
    {
      // Sword attack
      SendDamageTo( AttackTarget );
    }
  }
}

void AGameObject::Shoot()
{
  //info( FS( "%s is shooting a %s to %s", *Stats.Name, *GetTypesName(Stats.ReleasedProjectileWeapon),
  //  *AttackTarget->Stats.Name ) );
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

  LOG( "%s launching a projectile of type %d", *Stats.Name, (int)Stats.ReleasedProjectileWeapon.GetValue() );
  AProjectile* projectile = Game->Make<AProjectile>( Stats.ReleasedProjectileWeapon, launchPos, team );
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
      projectile->BaseStats.SpeedMax, projectile->BaseStats.MaxTravelHeight );
  }
}

void AGameObject::SendDamageTo( AGameObject* go )
{
  float damage = DamageRoll() - go->Stats.Armor;
  info( FS( "%s attacking %s for %f damage", *Stats.Name, *go->Stats.Name, damage ) );
  go->Hp -= damage;
  if( go->Hp < 0 )
    go->Hp = 0.f;
}

void AGameObject::ApplyEffect( FUnitsDataRow item )
{
  // 1-frame application of effect.
  Hp += item.HpMax; // This field contains hp boosts.
}

// Function adds a buff of type for specified time interval
void AGameObject::AddBuff( Types item )
{
  FUnitsDataRow itemData = Game->GetData( item );

  // TimeLength, dataSet
  LOG( "Applying %s for %f seconds", *itemData.Name, itemData.TimeLength );

  // Don't do anything for the Nothing item
  if( IsItem( itemData.Type ) )
  {
    // If there's no timeout, (0.0) then it applies for one frame only (hp boost +250 hp eg
    // applies next frame).
    BonusTraits.push_back( PowerUpTimeOut( itemData, itemData.TimeLength ) );
  }
  else
  {
    LOG( "%s NOT AN ITEM", *itemData.Name );
  }
}

void AGameObject::UpdateStats( float t )
{
  Stats = BaseStats;
  for( int i = 0; i < BonusTraits.size(); i++ )
    Stats += BonusTraits[i].traits;

  // Recover HP at stock recovery rate
  if( Repairing ) {
    Hp += Stats.RepairHPFractionCost*t;
    Clamp( Hp, 0.f, Stats.HpMax );
  }

  // Check buffs
  for( int i = 0; i < BonusTraits.size(); i++ )
  {
    // Apply any per-frame effects to the unit in bonus traits
    ApplyEffect( BonusTraits[i].traits );
  }
  
  // Tick all the traits
  for( int i = BonusTraits.size() - 1; i >= 0; i-- ) {
    BonusTraits[i].timeRemaining -= t;
    if( BonusTraits[i].timeRemaining <= 0 )
      BonusTraits.erase( BonusTraits.begin() + i );
  }

  // tick builds, and remove finished counters
  for( int i = BuildQueueCounters.size() - 1; i >= 0; i-- )
  {
    BuildQueueCounters[i].Time += t;
    // update the viz of the build queue counter, ith clock
    if( BuildQueueCounters[i].Done() )
    {
      // Remove it and consider ith building complete. Place @ unoccupied position around building.
      FVector buildPos = Pos + GetActorForwardVector() * GetBoundingRadius();
      
      AGameObject* newUnit = Game->Make<AGameObject>( BuildQueueCounters[i].Type, buildPos, team );

      // Remove counter & refresh the build queue.
      removeIndex( BuildQueueCounters, i );
      Game->hud->ui->gameChrome->buildQueue->Refresh();
    }
  }
}

bool AGameObject::UseAbility( int index )
{ 
  if( index < 0 || index > Stats.Abilities.Num() )
  {
    error( FS( "%s cannot use ability %d, OOB", *Stats.Name, index ) );
    return 0;
  }

  Types type = Stats.Abilities[index];
  if( IsAction( type ) )
  {
    FUnitsDataRow action = Game->GetData( type );
    info( FS( "%s used action %s", *Stats.Name, *action.Name ) );
  }
  else if( IsBuilding( type ) )
  {
    info( FS( "Building a %s", *GetTypesName( type ) ));
    // Set placement object with instance of type
    Game->flycam->ghost = Game->Make< ABuilding >( type, team );

  }
  else if( IsUnit( type ) )
  {
    // makes unit of type
    info( FS( "Making a unit of type %s", *GetTypesName( type ) ));
    Make( type );
  }
  return 1;
}

bool AGameObject::Make( Types type )
{
  // Start building.
  BuildQueueCounters.push_back( CooldownCounter( type ) );

  Game->hud->ui->gameChrome->buildQueue->Refresh(); // enqueue a refresh of the build queue

  return 1;
}

void AGameObject::SetRot( const FRotator & ro )
{
  if( RootComponent )
  {
    RootComponent->SetWorldRotation( ro );
  }
  else error( "No root component" );
}

bool AGameObject::Reached( FVector& v, float dist )
{
  FVector diff = Pos - v;
  diff.Z = 0;
  return diff.Size() < dist;
}

void AGameObject::CheckWaypoint()
{
  // How much of the way are we towards our next destination point
  // Check Waypoints. If reached dest, then pop next waypoint.
  if( Reached( Dest, WaypointReachedToleranceDistance ) )
  {
    // Pop the next waypoint.
    if( !Waypoints.size() )
      return;
    Dest = Waypoints.front();
    pop_front( Waypoints );
  }
}

void AGameObject::SetPosition( FVector v )
{
  // Check if there's something there.
  //set<AGameObject*> objs = Game->pc->Pick( 

  // While encountering objects, keep searching adjacent to the picked objects until finding empty place.
  Pos = Dest = v;
}

void AGameObject::FlushPosition()
{
  // Flush the computed position to the root component
  RootComponent->SetWorldLocation( Pos );
}

// Other gameobject is too close, so a repulsion force is added
// proportional with formula around ln( a - x ) like formula
FVector AGameObject::Repel( AGameObject* go )
{
  // The object will overlap in the future position, so don't move.
  // Get radius of other actor
  float r = GetBoundingRadius() + go->GetBoundingRadius();
  FVector from = Pos - go->Pos;
  if( float x = from.Size() )
  {
    from /= x;
    if( x < 1 + r )
    {
      return RepelMultiplier * SpeedPercent()/100.f * 
        log( 1 + r - x )/log( 1 + r ) * from;
    }
    else
    {
      LOG( "x is too large (%f) for radius r=%f", x, r );
    }
  }

  return FVector(0,0,0);
}

void AGameObject::OnHitContactBegin_Implementation( AActor* OtherActor,
  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
  bool bFromSweep, const FHitResult & SweepResult )
{
  //LOG( "OnHitContactBegin %s with %s", *Stats.Name, *OtherActor->GetName() );

  if( OtherActor == this )
  {
    // Don't do anything with reports of collision with self.
    return;
  }

  AProjectile *p = Cast<AProjectile>( this );
  AGameObject *other = Cast<AGameObject>( OtherActor );
  // Make sure other component with defined overlap is AGameObject derivative.
  if( other && p )
  {
    // If this object type detonates on contact, then its a projectile.
    // make it explode when it hits its attack target
    if( AttackTarget == other || other == Game->flycam->floor )
    {
      // This means the object is a projectile.
      if( AttackTarget && !AttackTarget->Dead )
      {
        LOG( "%s is detonating", *Stats.Name );
        // Damage the attack target with impact-damage
        SendDamageTo( AttackTarget );
      }
      else if( other == Game->flycam->floor )
      {
        LOG( "%s is contacting the floor", *Stats.Name );
      }
      
      if( Stats.OnContact )
      {
        AGameObject* expl = Game->Make<AGameObject>( Stats.OnContact, GetTip(), team );
      }

      Die();
      Destroy();
    }
  }
}

void AGameObject::OnHitContactEnd_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
  //LOG( "OnHitContactEnd" );
}

void AGameObject::OnRepulsionContactBegin_Implementation( AActor* OtherActor,
  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
  bool bFromSweep, const FHitResult & SweepResult )
{
  //LOG( "OnRepulsionContactBegin" );

  if( OtherActor == this )
  {
    // Don't do anything with reports of collision with self.
    return;
  }

  // Make sure other component with defined overlap is AGameObject derivative.
  if( AGameObject *go = Cast<AGameObject>( OtherActor ) )
  {
    Overlaps.push_back( go );
  }
}

void AGameObject::OnRepulsionContactEnd_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
  //LOG( "OnRepulsionContactEnd" );
  
  if( AGameObject *go = Cast<AGameObject>( OtherActor ) )
  {
    removeElement( Overlaps, go );
  }
}

// Adds repulsion forces from any adjacent gameobjects (to avoid collisions)
void AGameObject::AddRepulsionForcesFromOverlappedUnits()
{
  FVector forces( 0,0,0 );
  // If there is no attack target, don't use repel forces.
  for( int i = 0; i < Overlaps.size(); i++ )
  {
    FVector repel = Repel( Overlaps[i] );
    forces += repel; // gather repelling force from other object
  }
  // Add in repulsion forces to the position.
  Pos += forces;
}

void AGameObject::Walk( float t )
{
  CheckWaypoint();

  // Alter destination based on locations of other units.
  FVector ToDest = Dest - Pos;
  
  // Clamp travel length so that we can't overshoot destination
  if( float Len = ToDest.Size() )
  {
    Dir = ToDest / Len; // normalize
    if( !Stats.SpeedMax )  error( FS("%s had 0 speed", *GetName()) );
    
    Speed = Stats.SpeedMax;
    Vel = Dir*Speed;
    FVector travel = Vel*t;

    // If travel exceeds destination, then jump to dest,
    // so we don't jitter over the final position.
    if( Len < travel.Size() )
    {
      // snap to position & stop moving.
      Pos = Dest; // we are @ destination.
      travel = ToDest; // This is the displacement we actually moved.
      Speed = 0;
      Vel = FVector( 0, 0, 0 );
      if( OnReachDestination ) {
        OnReachDestination(); // Execute destination reached function
        OnReachDestination = function< void () >();
      }
    }
    else
    {
      Pos += travel;
      SetRot( Dir.Rotation() );
    }

    // Push UP from the ground plane, using the bounds on the actor.
    if( !Game->flycam->SetOnGround( Pos ) )
    {
      LOG( "object %s has left the ground plane", *Stats.Name );
    }

    if( !AttackTarget )
      AddRepulsionForcesFromOverlappedUnits();
  }
}

void AGameObject::SetGroundPosition( FVector groundPos )
{
  StopAttackAndFollow();
  SetDestination( groundPos );
}

void AGameObject::SetDestination( FVector d )
{
  //LOG( "%s moving from %f %f %f to %f %f %f", *Stats.Name, Pos.X, Pos.Y, Pos.Z, d.X, d.Y, d.Z );
  if( !Stats.SpeedMax ) {
    error( FS( "%s warning: Set unit's destination on unit with SpeedMax=0", *Stats.Name ) );
    return;
  }

  // Make sure the destination is grounded
  Game->flycam->SetOnGround( d );
  
  // Find the path, then submit list of Waypoints
  Waypoints = Game->flycam->pathfinder->findPath( Pos, d );
  // Fix Waypoints z value so they sit on ground plane
  for( int i = 0; i < Waypoints.size(); i++ )
  {
    if( !Game->flycam->SetOnGround( Waypoints[i] ) ) // Then set on ground.
    {
      LOG( "Waypoint %f %f %f couldn't reach ground",
        Waypoints[i].X, Waypoints[i].Y, Waypoints[i].Z );
    }
  }

  //Game->flycam->ClearViz();
  
  // Check that the 2nd point isn't more than 90
  // degrees away from the 1st point.
  if( Waypoints.size() >= 3 )
  {
    FVector b1 = Waypoints[ Waypoints.size() - 1 ];
    FVector b2 = Waypoints[ Waypoints.size() - 2 ];
    FVector b3 = Waypoints[ Waypoints.size() - 3 ];

    // The two vectors between (b3, b2) and (b2, b1) must not have a large angle between them.
    // If they do, delete the 2nd from the back one.
    //   1 <--- 2 <--- 3  [ OK ]
    //
    //   1 <--- 2 [ cut middle elt ]
    //         ^
    //        /
    //       /
    //      3
    FVector dir1 = b2 - b3;
    FVector dir2 = b1 - b2;
    dir1.Normalize(), dir2.Normalize();
    const float a = cosf( WaypointAngleTolerance );
    float dot = FVector::DotProduct( dir1, dir2 );
    if( dot < a )
    {
      // Pop the 2nd from the back point, viz all back 3 pts.
      vector<FVector>::iterator it = --(--(Waypoints.end()));
      //Game->flycam->Visualize( Types::UNITSPHERE, *it, 64.f, FLinearColor::Black );
      Waypoints.erase( it );
    }
  }

  //Game->flycam->Visualize( Types::UNITSPHERE, Waypoints, 64.f, FLinearColor::Green, FLinearColor::Red );
  // Visualize the pathway
  //Game->flycam->ClearViz();
  //Game->flycam->Visualize( UNITSPHERE, Waypoints, 11.f, FLinearColor( 0, 0, 0, 1.f ), FLinearColor( 1, 1, 1, 1.f ) );
  //Game->flycam->Visualize( d, 44.f, FLinearColor::Red );
  
  Dest = Waypoints.front();
  pop_front( Waypoints );
}

void AGameObject::StopMoving()
{
  Waypoints.clear(); // clear the Waypoints
  Dest = Pos; // You are at your destination
}

void AGameObject::Stop()
{
  StopMoving();
  StopAttackAndFollow();
}

void AGameObject::Face( FVector point )
{
  FVector to = point - Pos;
  if( float len = to.Size() )
  {
    to /= len;
    SetRot( to.Rotation() );
  }
}

// Stand outside target within `distance` units
void AGameObject::MoveWithinDistanceOf( AGameObject* target, float distance )
{
  FVector targetToMe = Pos - target->Pos;
  float len = targetToMe.Size();
  //info( FS( "%s attacking %s is %f units from it", *Stats.Name, *AttackTarget->Stats.Name, len ) );
  // If we're outside the attack range.. move in.
  if( len > distance )
  {
    targetToMe /= len;
    // set the fallback distance to being size of bounding radius of other unit
    SetDestination( target->Pos + targetToMe*Stats.AttackRange*.9f );
  }
  else
  {
    // You are within attack range, so face the attack target
    Face( target->Pos );
  }
}

void AGameObject::Move( float t )
{
  if( Dead ) {
    //error( FS( "Dead Unit %s had Move called for it", *Stats.Name ) );
    return; // Cannot move if dead
  }
  if( Hp <= 0 ) {
    Die();
    return;
  }
  else {
    UpdateStats( t );
    // Update & Cache Unit's stats this frame, including HP recovery
  }
  
  // Call the ai for this object type
  //ai( t );
  FlushPosition();
}

void AGameObject::ai( float t )
{
  // Base GameObject doesn't have AI for it.
}

bool AGameObject::isAllyTo( AGameObject* go )
{
  return team->isAllyTo( go ); // Check with my team
}

bool AGameObject::isEnemyTo( AGameObject* go )
{
  return team->isEnemyTo( go );
}

void AGameObject::Follow( AGameObject* go )
{
  StopAttackAndFollow();
  FollowTarget = go;
  if( FollowTarget )
  {
    FollowTarget->Followers.push_back( this );
    Game->hud->MarkAsFollow( FollowTarget );
  }
}

void AGameObject::Attack( AGameObject* go )
{
  StopAttackAndFollow();
  AttackTarget = go;
  if( AttackTarget )
  {
    AttackTarget->Attackers.push_back( this );
    Game->hud->MarkAsAttack( AttackTarget );
  }
}

void AGameObject::StopAttackAndFollow()
{
  // Tell my old follow target (if any) that I'm no longer following him
  if( FollowTarget )
  {
    //LOG( "%s losing follower %s", *Stats.Name, *FollowTarget->Stats.Name );
    FollowTarget->LoseFollower( this );
  }

  // If the AttackTarget was already set, tell it loses the old attacker (this).
  if( AttackTarget )
  {
    //LOG( "%s losing follower %s", *Stats.Name, *AttackTarget->Stats.Name );
    AttackTarget->LoseAttacker( this );
  }
}

void AGameObject::LoseFollower( AGameObject* formerFollower )
{
  if( !formerFollower ) error( "Cannot lose null follower" );
  removeElement( Followers, formerFollower );
  formerFollower->FollowTarget = 0;
  // if I lost all followers, update the hud
  if( !Followers.size() ) {
    LOG( "%s doesn't need follow ring", *Stats.Name );
    RemoveTagged( this, Game->hud->FollowTargetName );
  }
}

void AGameObject::LoseAttacker( AGameObject* formerAttacker )
{
  if( !formerAttacker ) error( "Cannot lose null follower" );
  removeElement( Attackers, formerAttacker );
  formerAttacker->AttackTarget = 0;
  // If there are no more attackers, unselect in ui
  if( !Attackers.size() )
  {
    LOG( "%s doesn't need attack ring", *Stats.Name );
    RemoveTagged( this, Game->hud->AttackTargetName );
  }
}

void AGameObject::LoseAttackersAndFollowers()
{
  for( int i = Attackers.size()-1; i >= 0; i-- )
    Attackers[i]->Attack( 0 );
  for( int i = Followers.size()-1; i >= 0; i-- )
    Followers[i]->Follow( 0 );
}

AGameObject* AGameObject::GetClosestEnemyUnit()
{
  map<float, AGameObject*> closeUnits = FindEnemyUnitsInSightRange();
  if( closeUnits.size() )
    return closeUnits.begin()->second;
  return 0;
}

map<float, AGameObject*> AGameObject::FindEnemyUnitsInSightRange()
{
  map< float, AGameObject* > distances;
	for( Team* otherTeam : Game->gm->teams )
  {
    if( ! otherTeam->isEnemyTo( this ) ) skip;

    for( AGameObject* go : otherTeam->units )
    {
      // Cannot Target unit on same team (also prevents targetting self)
      float dist = FVector::Dist( go->Pos, Pos );
      if( dist < Stats.SightRange )
        distances[ dist ] = go;
    }
  }
  
  //if( !distances.size() )  info( "No enemies to attack, you may be victorious" );
  return distances;
}

AGameObject* AGameObject::GetClosestObjectOfType( Types type )
{
  AGameObject* closestObject = 0;
  float closestDistance = FLT_MAX;

  // Get all objects in the level of this type
  ULevel* level = GetLevel();
  TTransArray<AActor*> *actors = &level->Actors;
  for( int i = 0; i < actors->Num(); i++ )
  {
    AGameObject* g = Cast<AGameObject>( (*actors)[i] );
    if( g   &&   g->Stats.Type == type )
    {
      float dist = FVector::Dist( Pos, g->Pos );
      if( dist < closestDistance ) {
        closestObject = g;
        closestDistance = dist;
      }
    }
  }

  return closestObject;
}

void AGameObject::OnSelected()
{
  // play selection sound
  if( Greets.Num() )  PlaySound( Greets[ randInt( Greets.Num() ) ].Sound );
}

float AGameObject::GetBoundingRadius()
{
  FVector2D size;
  GetComponentsBoundingCylinder( size.X, size.Y, 1 );
  return size.GetMax();
}

FBox AGameObject::GetBoundingBox()
{
  return GetComponentsBoundingBox();
}

FCollisionShape AGameObject::GetBoundingCylinder()
{
  FCollisionShape c1;
  c1.ShapeType = ECollisionShape::Capsule;
  GetSimpleCollisionCylinder( c1.Capsule.Radius, c1.Capsule.HalfHeight );
  return c1;
}

void AGameObject::SetMaterialColors( FName parameterName, FLinearColor color )
{
  // Grab all meshes with material parameters & set color of each
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>();
  for( UMeshComponent* mesh : meshes )
  {
    for( int i = 0; i < mesh->GetNumMaterials(); i++ )
    {
      UMaterialInterface *mi = mesh->GetMaterial( i );
      if( UMaterialInstanceDynamic *mid = Cast< UMaterialInstanceDynamic >( mi ) )
      {
        info( "The MID was created" );
        mid->SetVectorParameterValue( FName( parameterName ), color );
      }
      else
      {
        //info( "The MID wasn't created" );
        mid = UMaterialInstanceDynamic::Create( mi, this );
        FLinearColor defaultColor;
        if( mid->GetVectorParameterValue( parameterName, defaultColor ) )
        {
          mid->SetVectorParameterValue( parameterName, color );
          mesh->SetMaterial( i, mid );
          //info( FS( "Setting mid param %s", *(parameterName.ToString()) ) );
        }
      }
    }
  }
}

void AGameObject::SetTeam( Team* newTeam )
{
  // If the old team this unit was on is set, remove it from that team.
  if( team ) { team->RemoveUnit( this ); }

  // change team. There is no AddUnit function in Team because more _this function_
  // must be called. To avoid cyclic SetTeam->AddUnit->SetTeam calls, we only supply
  // a SetTeam function here.
  team = newTeam;
  team->units.push_back( this );
  BaseStats.TeamId = Stats.TeamId = team->teamId;
  
  // Grab all meshes with material parameters & set color of each
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>();
  for( UMeshComponent* mesh : meshes )
  {
    for( int i = 0; i < mesh->GetNumMaterials(); i++ )
    {
      UMaterialInterface *mi = mesh->GetMaterial( i );
      if( UMaterialInstanceDynamic *mid = Cast< UMaterialInstanceDynamic >( mi ) )
      {
        info( "The MID was created " );
        mid->SetVectorParameterValue( FName( "TeamColor" ), team->Color );
      }
      else
      {
        //info( "The MID wasn't created " );
        mid = UMaterialInstanceDynamic::Create( mi, this );
        FLinearColor defaultColor;
        if( mid->GetVectorParameterValue( FName( "TeamColor" ), defaultColor ) )
        {
          mid->SetVectorParameterValue( FName( "TeamColor" ), team->Color );
          mesh->SetMaterial( i, mid );
          //info( "Setting mid param" );
        }
      }
    }
  }
}

void AGameObject::SetMaterial( UMaterialInterface* mat )
{
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>();
    for( int i = 0; i < meshes.size(); i++ )
      for( int j = 0; j < meshes[i]->GetNumMaterials(); j++ )
        meshes[i]->SetMaterial( j, mat );
}

void AGameObject::SetColor( FLinearColor color )
{
  SetMaterial( Game->flycam->GetMaterial( color ) );
}

void AGameObject::Die()
{
  StopAttackAndFollow(); // Remove its attack and follow
  LoseAttackersAndFollowers(); // Attackers and followers stop detecting

  Dead = 1; // updates the blueprint animation,

  // Filter THIS from collection if exists
  removeElement( Game->hud->Selected, this );

  // Remove from team. This finally removes its game-tick counter.
  if( team )  team->RemoveUnit( this );
}

void AGameObject::BeginDestroy()
{
  // For odd-time created objects (esp in PIE) they get put into the team without ever actually
  // being played with, so they don't die properly.
  if( team ) {
    //warning( FS( "Unit %s was removed from team in BeginDestroy()", *Stats.Name ) );
    team->RemoveUnit( this );
  }

  // Check if object is selected, only possible game launched/ready
  //info( FS( "%s was destroyed", *Stats.Name ) );
  
  Super::BeginDestroy(); // PUT THIS LAST or the object may become invalid
}

