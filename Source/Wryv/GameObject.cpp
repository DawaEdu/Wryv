#include "Wryv.h"
#include "Building.h"
#include "Explosion.h"
#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "GameObject.h"
#include "TheHUD.h"
#include "Pathfinder.h"
#include "Peasant.h"
#include "PlayerControl.h"
#include "Projectile.h"
#include "Widget3D.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

const float AGameObject::WaypointAngleTolerance = 30.f; // 
const float AGameObject::WaypointReachedToleranceDistance = 250.f; // The distance to consider waypoint as "reached"

AGameObject* AGameObject::Nothing = 0;
float AGameObject::CapDeadTime = 10.f;

// Sets default values
AGameObject::AGameObject( const FObjectInitializer& PCIP )
{
  //LOG( "%s [%s]->AGameObject::AGameObject()", *GetName(), *BaseStats.Name );
  PrimaryActorTick.bCanEverTick = true;
  ID = 0;
  Repairing = 0;
  team = 0;
  Pos = Vel = FVector(0, 0, 0);
  FollowTarget = AttackTarget = 0;
  RepelMultiplier = 1.f;
  Dead = 0;
  DeadTime = 0.f;
  MaxDeadTime = CapDeadTime;

  DummyRoot = PCIP.CreateDefaultSubobject<USceneComponent>( this, "Dummy" );
  SetRootComponent( DummyRoot );
  hitBounds = PCIP.CreateDefaultSubobject<UCapsuleComponent>( this, "HitVolumex222" );
  hitBounds->AttachTo( DummyRoot );
  repulsionBounds = PCIP.CreateDefaultSubobject<USphereComponent>( this, "RepulsionVolumex22" );
  repulsionBounds->AttachTo( DummyRoot );
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

  ID = Game->NextId();
}

void AGameObject::OnMapLoaded()
{
  
}

float AGameObject::Hash()
{
  // The Hash function computes a simple checksum of the object, combining
  // ID of what object 
  return Pos.X + Pos.Y + Pos.Z + ID + 
         (FollowTarget?FollowTarget->Hash():0)+
         (AttackTarget?AttackTarget->Hash():0);
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
  float maxDimen = GetComponentsBoundingBox().GetSize().GetMax();
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

float AGameObject::outerDistance( AGameObject *go )
{
  if( !go ) {
    LOG( "outerDistance( 0 ): null" );
    return FLT_MAX;
  }
  float dist = centroidDistance( go );
  return dist - (Radius() + go->Radius());
}

bool AGameObject::isAttackTargetWithinRange()
{
  if( AttackTarget )
  {
    return centroidDistance( AttackTarget ) < Stats.AttackRange;
  }

  return 0;
}

float AGameObject::HpFraction()
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
  AProjectile* projectile = Game->Make<AProjectile>( Stats.ReleasedProjectileWeapon, team, launchPos );
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
  info( FS( "%s melee attacking %s for %f damage", *Stats.Name, *go->Stats.Name, damage ) );
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
      FVector buildPos = Pos + GetActorForwardVector() * Radius();
      
      AGameObject* newUnit = Game->Make<AGameObject>( BuildQueueCounters[i].Type, team, buildPos );

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
  if( IsBuilding( type ) )
  {
    info( FS( "Building a %s", *GetTypesName( type ) ));
    // Set placement object with instance of type
    Game->flycam->ghost = Game->Make< ABuilding >( type, team );
  }
  else if( IsAction( type ) )
  {
    FUnitsDataRow action = Game->GetData( type );
    info( FS( "%s used action %s", *Stats.Name, *action.Name ) );
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
  // Can check if there's something there.
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
  float r = 1.f;
  if( AttackTarget == go )
    // For attack target, use the hitBounds for the radius
    r = repulsionBounds->GetScaledSphereRadius()   +   go->repulsionBounds->GetScaledSphereRadius();
  else
    r = hitBounds->GetScaledCapsuleHalfHeight()   +   go->hitBounds->GetScaledCapsuleHalfHeight();

  // The object will overlap in the future position, so don't move.
  // Get radius of other actor
  FVector from = Pos - go->Pos;
  if( float x = from.Size() )
  {
    from /= x;
    if( x < 1.f + r )
    {
      return RepelMultiplier * SpeedPercent()/100.f * 
        log( 1.f + r - x )/log( 1.f + r ) * from;
    }
    else
    {
      //LOG( "x is too large (%f) for radius r=%f", x, r );
    }
  }

  return FVector(0,0,0);
}

void AGameObject::OnHitContactBegin_Implementation( AActor* OtherActor,
  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
  bool bFromSweep, const FHitResult & SweepResult )
{
  info( FS( "OnHitContactBegin %s with %s", *Stats.Name, *OtherActor->GetName() ) );
  if( OtherActor == this )
  {
    // Don't do anything with reports of collision with self.
    return;
  }

  AGameObject* THIS = Cast<AGameObject>( this );
  AGameObject* THAT = Cast<AGameObject>( OtherActor );
  if( THIS && THAT )
  {
    // Both were gameobjects
    THIS->Hit( THAT );
  }
  else
  {
    error( "One of the colliding objects was not a gameobject instance" );
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
    Overlaps += go;
  }
}

void AGameObject::OnRepulsionContactEnd_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
  //LOG( "OnRepulsionContactEnd" );
  
  if( AGameObject *go = Cast<AGameObject>( OtherActor ) )
  {
    Overlaps -= go;
  }
}

// Adds repulsion forces from any adjacent gameobjects (to avoid collisions)
void AGameObject::AddRepulsionForcesFromOverlappedUnits()
{
  FVector forces( 0,0,0 );

  // If there is no attack target, don't use repel forces.
  for( AGameObject * go : Overlaps )
  {
    FVector repel = Repel( go );
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
    }
    else
    {
      Pos += travel;
      SetRot( Dir.Rotation() );
    }

    // Push UP from the ground plane, using the bounds on the actor.
    FVector AboveGround = Pos;
    AboveGround.Z += 20.f;
    if( Game->flycam->SetOnGround( AboveGround ) )
    {
      Pos = AboveGround;
    }
    else
    {
      LOG( "object %s has left the ground plane", *Stats.Name );
    }

    if( !AttackTarget )
      AddRepulsionForcesFromOverlappedUnits();
  }
}

void AGameObject::GoToGroundPosition( FVector groundPos )
{
  DropAttackAndFollowTargets();
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
  if( !Waypoints.size() )
  {
    error( FS( "No waypoints in path from %f %f %f to %f %f %f", Pos.X, Pos.Y, Pos.Z, d.X, d.Y, d.Z ) );
    Dest = d;
    return;
  }

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
  DropAttackAndFollowTargets();
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
void AGameObject::MoveWithinDistanceOf( AGameObject* target, float fallbackDistance )
{
  FVector targetToMe = Pos - target->Pos;
  float len = targetToMe.Size();
  //info( FS( "%s attacking %s is %f units from it", *Stats.Name, *AttackTarget->Stats.Name, len ) );
  // If we're outside the attack range.. move in.
  if( len > fallbackDistance )
  {
    targetToMe /= len;
    // set the fallback distance to being size of bounding radius of other unit
    SetDestination( target->Pos + targetToMe*fallbackDistance );
  }
  else
  {
    // Within distance, so face
    // You are within attack range, so face the attack target
    Face( target->Pos );
  }
}

void AGameObject::DisplayWaypoints()
{
  for( int i = 0; i < commands.size(); i++ )
  {
    if( commands[i].commandType == Command::CommandType::CreateBuilding )
    {
      //TODO: Ghost the building.
    }
    else
    {
      // Stick a flag in
      AShape *flag = Game->Make<AShape>( Types::UIFLAGWAYPOINT, team, commands[i].pos );
      Game->Flags[ commands[i].CommandID ] = flag;
      flag->text = FS( "%d", i+1 );
    }
  }
}

void AGameObject::exec( const Command& cmd )
{
  info( FS( "Executing command: %s", *cmd.ToString() ) );
  // remove flag for previous cmd if any
  Game->ClearFlag( CurrentCommand.CommandID );
  CurrentCommand = cmd;
  AGameObject* go = Game->GetUnitById( cmd.srcObjectId );
  switch( cmd.commandType )
  {
    case Command::CommandType::Target:
      {
        AGameObject* target = Game->GetUnitById( cmd.targetObjectId );
        if( !go || !target )
        {
          error( "CommandType::Target: GameObject was NULL or target was NULL" );
        }
        else
        {
          go->Target( target );
        }
      }
      break;
    case Command::CommandType::GoToGroundPosition:
      {
        go->GoToGroundPosition( cmd.pos );
      }
      break;
    case Command::CommandType::CreateBuilding:
      {
        // builds the assigned building using peasant (id)
        if( APeasant* peasant = Cast<APeasant>( go ) )
        {
          info( FS( "The unit [%s] is building a %s",
            *peasant->GetName(), *GetTypesName( (Types)cmd.targetObjectId ) ) );
          peasant->Build( (Types)cmd.targetObjectId, cmd.pos );
        }
        else
        {
          error( FS( "The unit [%s] asked to build %s was not a peasant",
            *go->GetName(), *GetTypesName( (Types)cmd.targetObjectId ) ) );
        }
      }
      break;
    case Command::CommandType::RepairBuilding:
      {
        //p->SetDestination( Pos );
        // What if the peasant dies?
        //p->OnReachDestination = [this,p]()
        //{
        //  // put the peasant underground, so it appears to be building.
        //  peasant = p; // only set the peasant here, so building only starts once he gets there.
        //  peasant->SetPosition( peasant->Pos - FVector( 0, 0, peasant->GetHeight() ) );
        //};
      }
      break;
    case Command::CommandType::UseAbility:
      {
        go->UseAbility( cmd.targetObjectId );

      }
      break;
    default:
      error( FS( "Undefined command %d", (int)cmd.commandType ) );
      break;
  }
}

void AGameObject::Move( float t )
{
  if( Dead ) {
    //error( FS( "Dead Unit %s had Move called for it", *Stats.Name ) );
    DeadTime += t;
    if( DeadTime >= MaxDeadTime )
    {
      warning( FS( "Dead unit %s was cleaned up", *Stats.Name ) );
      Cleanup();
    }
    return; // Cannot move if dead
  }

  // Process enqueued commands when idling.
  if( Idling() )
  {
    if( commands.size() )
    {
      Command cmd = commands.front();
      // erase the flag for what we're doing next
      exec( cmd );
      commands.pop_front();
    }
    else
    {
      // Finished last task, idling.
      // Clean up last flag when get there
      Game->ClearFlag( CurrentCommand.CommandID );
    }
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

bool AGameObject::Idling()
{
  // no scheduled activity. sitting beside followtarget doesn't mean idling
  return 
    // Not attacking something
    !AttackTarget   &&
    // Within Radius() units of the set destination
    FVector::PointsAreNear( Pos, Dest, Radius() )   &&
    // No more sceduled waypoints of destination
    !Waypoints.size()
  ;
}

void AGameObject::ai( float t )
{
  // Base GameObject doesn't have AI for it.
}

// The hit volumes overlapped
void AGameObject::Hit( AGameObject* other )
{
  
}

float AGameObject::Radius()
{
  return hitBounds->GetScaledCapsuleRadius();
}

bool AGameObject::isAllyTo( AGameObject* go )
{
  return team->isAllyTo( go ); // Check with my team
}

bool AGameObject::isEnemyTo( AGameObject* go )
{
  return team->isEnemyTo( go );
}

void AGameObject::Target( AGameObject* target )
{
  if( isEnemyTo( target ) )
    Attack( target );
  else
    Follow( target );
}

void AGameObject::Follow( AGameObject* go )
{
  DropAttackAndFollowTargets();
  FollowTarget = go;
  if( FollowTarget )
  {
    if( FollowTarget->Dead ) {
      error( FS( "Trying to follow dead attack target %s", *go->Stats.Name ) );
      return;
    }
    
    FollowTarget->Followers += this;
    Game->hud->MarkAsFollow( FollowTarget );
  }
}

void AGameObject::Attack( AGameObject* go )
{
  DropAttackAndFollowTargets();
  AttackTarget = go;
  if( AttackTarget )
  {
    if( AttackTarget->Dead ) {
      error( FS( "Trying to attack dead attack target %s", *go->Stats.Name ) );
      return;
    }

    AttackTarget->Attackers += this;
    Game->hud->MarkAsAttack( AttackTarget );
  }
}


void AGameObject::DropAttackAndFollowTargets()
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
    //LOG( "%s losing attacker %s", *Stats.Name, *AttackTarget->Stats.Name );
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
    //LOG( "%s doesn't need follow ring", *Stats.Name );
    RemoveTagged( this, Game->hud->FollowTargetName );
  }
}

void AGameObject::LoseAttacker( AGameObject* formerAttacker )
{
  if( !formerAttacker ) error( "Cannot lose null follower" );
  // This tends to be called while iterating. So be careful here.
  removeElement( Attackers, formerAttacker );
  
  formerAttacker->AttackTarget = 0;
  // If there are no more attackers, unselect in ui
  if( !Attackers.size() )
  {
    //LOG( "%s doesn't need attack ring", *Stats.Name );
    RemoveTagged( this, Game->hud->AttackTargetName );
  }
}

void AGameObject::LoseAttackersAndFollowers()
{
  // Carefully remove each Attacker from the set. Since the
  // set CANNOT be removed from while iterating here, we cannot use
  // a regular range for based loop.
  //for( AGameObject* go : Attackers )  // Won't work, since go->Attack( 0 ) causes removal from Attackers set.
  //  go->Attack( 0 );                  // Won't work, since go->Attack( 0 ) causes removal from Attackers set.
  // Cap iterations @ Attackers.size(), to definite safe-guard against infinite loop bug.
  for( int i = Attackers.size() - 1; i >= 0; i-- )
    (*Attackers.begin())->Attack( 0 );
  if( Attackers.size() )
    error( FS( "%s: There are %d attackers after losing all attackers", *Stats.Name, Attackers.size() ) );

  for( int i = Followers.size() - 1; i >= 0; i-- )
    (*Followers.begin())->Follow( 0 );
  if( Followers.size() )
    error( FS( "%s: There are %d followers after losing all followers", *Stats.Name, Followers.size() ) );
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
  // You can only select within range of unit
  // Go thru all objects on team
  map< float, AGameObject* > m;
  for( AGameObject* go : team->units )
  {
    if( go->Stats.Type != type )  skip;

    float d = outerDistance( go );
    m[d] = go;
  }
  if( m.size() )
    return m.begin()->second;

  info( FS( "Could not find an object of type %s", *GetTypesName( type ) ) );
  return 0; // NO UNITS In sight
}

void AGameObject::OnSelected()
{
  // play selection sound
  if( Greets.Num() )  PlaySound( Greets[ randInt( Greets.Num() ) ].Sound );
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
        //info( "The MID was created" );
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
  DropAttackAndFollowTargets(); // Remove its attack and follow
  LoseAttackersAndFollowers(); // Attackers and followers stop detecting

  Dead = 1; // updates the blueprint animation and kicks off the death animation in the
  // state machine (usually).

  // Remove from selection.
  if( Game->hud ) Game->hud->Unselect( { this } );

  if( Stats.OnExploded )
  {
    Game->Make<AGameObject>( Stats.OnExploded, team, Pos );
  }
}

void AGameObject::Cleanup()
{
  // Remove from team. This finally removes its game-tick counter.
  if( team )
  {
    team->RemoveUnit( this );
  }

  if( Attackers.size() || Followers.size() )
  {
    error( FS("There are %d attackers and %d followers", Attackers.size(), Followers.size() ) );
    LoseAttackersAndFollowers();
  }

  Destroy();
}

void AGameObject::BeginDestroy()
{
  // For odd-time created objects (esp in PIE) they get put into the team without ever actually
  // being played with, so they don't die properly.
  if( team ) {
    //warning( FS( "Unit %s was removed from team in BeginDestroy()", *Stats.Name ) );
    team->RemoveUnit( this );
  }

  if( Attackers.size() || Followers.size() )
  {
    error( FS("There are %d attackers and %d followers", Attackers.size(), Followers.size() ) );
    LoseAttackersAndFollowers();
  }

  // Check if object is selected, only possible game launched/ready
  //info( FS( "%s was destroyed", *Stats.Name ) );
  
  Super::BeginDestroy(); // PUT THIS LAST or the object may become invalid
}

FString AGameObject::ToString()
{
  FString string = GetName();
  if( AttackTarget )
    string += FString("\nATT: ") + AttackTarget->GetName();
  if( FollowTarget )
    string += FString("\nFOL: ") + FollowTarget->GetName();
  return string;
}

FString AGameObject::FollowersToString()
{
  FString string = GetName();
  if( Attackers.size() )  string += FString( "[" );
  for( AGameObject* go : Attackers )
    string += go->GetName() + FString(", ");
  if( Attackers.size() )  string += FString( "]" );

  if( Followers.size() )  string += FString( "[" );
  for( AGameObject* go : Followers )
    string += go->GetName() + FString(", ");
  if( Followers.size() )  string += FString( "]" );
  return string;
}

