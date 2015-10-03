#include "Wryv.h"

#include "Building.h"
#include "Explosion.h"
#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "GameObject.h"
#include "Item.h"
#include "TheHUD.h"
#include "Pathfinder.h"
#include "Peasant.h"
#include "PlayerControl.h"
#include "Projectile.h"
#include "Widget3D.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

#include "Action.h"
#include "BuildAction.h"
#include "CastSpellAction.h"
#include "InProgressBuilding.h"
#include "InProgressUnit.h"
#include "ItemAction.h"
#include "TrainingAction.h"
#include "UnitAction.h"

const float AGameObject::WaypointAngleTolerance = 30.f; // 
const float AGameObject::WaypointReachedToleranceDistance = 250.f; // The distance to consider waypoint as "reached"

AGameObject* AGameObject::Nothing = 0;
float AGameObject::CapDeadTime = 10.f;

// Sets default values
AGameObject::AGameObject( const FObjectInitializer& PCIP )
{
  //LOG( "%s [%s]->AGameObject::AGameObject()", *GetName(), *Name );
  PrimaryActorTick.bCanEverTick = true;
  ID = 0;
  team = 0;
  Pos = Vel = FVector(0, 0, 0);
  FollowTarget = AttackTarget = 0;
  RepelMultiplier = 1.f;
  Dead = 0;
  DeadTime = 0.f;
  MaxDeadTime = CapDeadTime;
  vizColor = FLinearColor::MakeRandomColor();
  vizSize = 10.f;
  IsReadyToRunNextCommand = 0;
  AttackReady = 0;
  HoldingGround = 0;
  
  DummyRoot = PCIP.CreateDefaultSubobject<USceneComponent>( this, "Dummy" );
  SetRootComponent( DummyRoot );
  hitBounds = PCIP.CreateDefaultSubobject<UCapsuleComponent>( this, "HitVolumex222" );
  hitBounds->AttachTo( DummyRoot );
  repulsionBounds = PCIP.CreateDefaultSubobject<USphereComponent>( this, "RepulsionVolumex22" );
  repulsionBounds->AttachTo( DummyRoot );


}

void AGameObject::PostInitializeComponents()
{
  //LOG( "%s [%s]->AGameObject::PostInitializeComponents()", *GetName(), *Name );
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

  Recovering = 1;

}

// Called when the game starts or when spawned
void AGameObject::BeginPlay()
{
  Super::BeginPlay();

  //LOG( "%s [%s]->AGameObject::BeginPlay()", *GetName(), *Name );
  Team* newTeam = Game->gm->teams[ Stats.TeamId ];
  SetTeam( newTeam );

  ID = Game->NextId();
}

void AGameObject::OnMapLoaded()
{
  
}

void AGameObject::InitIcons()
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
  // The base is the bottom of the unit.
  return Pos + GetActorUpVector() * hitBounds->GetScaledCapsuleHalfHeight()*2.f;
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
    return outerDistance( AttackTarget ) <= Stats.AttackRange;
  }

  return 0;
}

bool AGameObject::isFollowTargetWithinRange()
{
  if( FollowTarget )
  {
    return outerDistance( FollowTarget ) <= Stats.FollowFallbackDistance;
  }

  return 0;
}

float AGameObject::HpFraction()
{
  if( ! Stats.HpMax ) {
    error( FS( "HpMax==0.f for %s", *Stats.Name ) );
    return 1.f;
  }
  else  return Hp / Stats.HpMax;  // if max hp not set, just return hp it has
}

float AGameObject::SpeedPercent()
{
  if( ! Stats.SpeedMax )
  {
    error( FS( "SpeedMax==0.f for %s", *Stats.Name ) );
    return 100.f;
  }
  return 100.f * Speed / Stats.SpeedMax;
}

void AGameObject::ReceiveAttack( AGameObject* from )
{
  float damage = from->DamageRoll() - Stats.Armor;
  info( FS( "%s melee attacking %s for %f damage",
    *from->Stats.Name, *Stats.Name, damage ) );
  Hp -= damage;
  if( Hp < 0 )
    Hp = 0.f;
}

void AGameObject::UpdateStats( float t )
{
  // Refreshes Stats completely
  Stats = BaseStats;
  for( int i = 0; i < BonusTraits.size(); i++ )
    Stats += BonusTraits[i].Powerup->Stats;

  // Recover HP at stock recovery rate
  if( Recovering ) {
    Hp += Stats.RepairRate*t;
    Clamp( Hp, 0.f, Stats.HpMax );
  }

  // Tick all the traits
  for( int i = BonusTraits.size() - 1; i >= 0; i-- ) {
    BonusTraits[i].timeRemaining -= t;
    if( BonusTraits[i].timeRemaining <= 0 )
      BonusTraits.erase( BonusTraits.begin() + i );
  }
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
  //info( FS( "OnHitContactBegin %s with %s", *Name, *OtherActor->GetName() ) );
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
    HitOverlaps += THAT; // Retain collection of objects i'm overlapping with
  }
  else
  {
    error( "One of the colliding objects was not a gameobject instance" );
  }
}

void AGameObject::OnHitContactEnd_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
  //LOG( "OnHitContactEnd" );
  if( AGameObject* THAT = Cast<AGameObject>( OtherActor ) )
  {
    HitOverlaps -= THAT;
  }
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
    RepulsionOverlaps += go;
  }
}

void AGameObject::OnRepulsionContactEnd_Implementation( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
  //LOG( "OnRepulsionContactEnd" );
  
  if( AGameObject *go = Cast<AGameObject>( OtherActor ) )
  {
    RepulsionOverlaps -= go;
  }
}

// Adds repulsion forces from any adjacent gameobjects (to avoid collisions)
void AGameObject::AddRepulsionForcesFromOverlappedUnits()
{
  FVector forces( 0,0,0 );

  // If there is no attack target, don't use repel forces.
  for( AGameObject * go : RepulsionOverlaps )
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
  //info( FS( "%s attacking %s is %f units from it", *Name, *AttackTarget->Name, len ) );
  // If we're outside the attack range.. move in.
  if( len < fallbackDistance )
  {
    // Within distance, so face
    // You are within attack range, so face the attack target
    Face( target->Pos );
  }
  else
  {
    targetToMe /= len;
    // set the fallback distance to being size of bounding radius of other unit
    SetDestination( target->Pos + targetToMe*(fallbackDistance*.997f) );
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
      AShape *flag = Game->Make<AShape>( Game->flycam->WaypointFlagClass, team, commands[i].pos );
      Game->Flags[ commands[i].CommandID ] = flag;
      flag->text = FS( "%d", i+1 );
    }
  }
}

void AGameObject::exec( const Command& cmd )
{
  info( FS( "Executing command: %s", *cmd.ToString() ) );
  AGameObject* go = Game->GetUnitById( cmd.srcObjectId );
  if( ID != cmd.srcObjectId )
  {
    error( FS( "Submitted command %d id's didn't match %d/%d",
      cmd.CommandID, ID, cmd.srcObjectId ) );
  }
  
  switch( cmd.commandType )
  {
    case Command::CommandType::CreateBuilding:
      {
        // builds the assigned building using peasant (id)
        if( APeasant* peasant = Cast<APeasant>( this ) )
        {
          UBuildAction* buildAction = peasant->Buildables[ cmd.targetObjectId ];
          info( FS( "The unit [%s] is building a %s @ %f %f %f",
            *peasant->GetName(), *buildAction->BuildingType->GetName(),
            cmd.pos.X,cmd.pos.Y,cmd.pos.Z ) );
          
          peasant->Build( buildAction, cmd.pos );
        }
        else
        {
          error( FS( "The unit [%s] asked to build entry [%d] was not a peasant",
            *GetName(), cmd.targetObjectId ) );
        }
      }
      break;
    case Command::CommandType::CreateUnit:
      {
        APeasant* peasant = Cast<APeasant>( this );
        peasant->UseBuild( cmd.targetObjectId ); // C++ command
      }
      break;
    case Command::CommandType::GoToGroundPosition:
      {
        GoToGroundPosition( cmd.pos );
      }
      break;
    case Command::CommandType::Stop:
      {
        Stop();
      }
      break;
    case Command::CommandType::HoldGround:
      {
        Stop();
        AttackReady = 1;   // Ready to attack any in-range units who are NOT AttackTarget.
        HoldingGround = 1; // Do not run out to engage nearby units, unless in range.
      }
      break;
    case Command::CommandType::Target:
      {
        AGameObject* target = Game->GetUnitById( cmd.targetObjectId );
        if( !target )
        {
          error( "CommandType::Target: GameObject was NULL or target was NULL" );
        }
        else
        {
          Target( target );
        }
      }
      break;
    case Command::CommandType::UseAbility:
      {
        AUnit* unit = Cast<AUnit>( this );
        unit->UseAbility( cmd.targetObjectId );
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
    //error( FS( "Dead Unit %s had Move called for it", *Name ) );
    DeadTime += t;
    if( DeadTime >= MaxDeadTime )
    {
      warning( FS( "Dead unit %s was cleaned up", *Stats.Name ) );
      Cleanup();
    }
    return; // Cannot move if dead
  }

  // If explicitly asked to DoNextCommand.
  if( IsReadyToRunNextCommand )
  {
    // When a command completes, remove the flag for it (if any) and pop it from the queue
    if( commands.size() )
    {
      // If there are still commands left, execute the next one.
      exec( GetCurrentCommand() );
      IsReadyToRunNextCommand = 0;
    }
  }

  if( Idling() )
  {
    // Once we are idling, we can pop the current command and try to exec the next one
    if( commands.size() )
    {
      Game->ClearFlag( GetCurrentCommand().CommandID );
      commands.pop_front(); // pop out the front item in the queue
    }
    IsReadyToRunNextCommand = 1;
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
    // No more scheduled waypoints of destination
    !Waypoints.size()
    // FollowTarget isn't checked, because if you have reached and are
    // standing near your FollowTarget, then you are still idling.
  ;
}

void AGameObject::ai( float t )
{
  // Base GameObject doesn't have AI for it.
}

// The hit volumes overlapped
void AGameObject::Hit( AGameObject* other )
{
  // These objects have overlapped.
}

float AGameObject::Height()
{
  return 2.f*hitBounds->GetScaledCapsuleHalfHeight();
}

float AGameObject::Radius()
{
  return hitBounds->GetScaledCapsuleRadius();
}

void AGameObject::GoToGroundPosition( FVector groundPos )
{
  Stop(); // DropTargets() & clear old Destination waypoints
  SetDestination( groundPos );
  AttackReady = 0; // Do NOT stop to engage enemy units
}

void AGameObject::AttackGroundPosition( FVector groundPos )
{
  Stop(); // DropTargets() & clear old Destination waypoints
  SetDestination( groundPos );
  AttackReady = 1;
}

void AGameObject::Target( AGameObject* target )
{
  if( target == self )
  {
    warning( FS( "%s tried to target itself", *Stats.Name ) );
    return; // Cannot target self.
  }

  if( !target ) // Targetting NULL just drops targets. It doesn't clear the old waypoint queue.
    DropTargets(); // This leaves the waypoint queue as it was.
    // Call Stop() if you don't want the unit to continue moving towards where it was.
  else if( isEnemyTo( target ) )
    Attack( target );
  else
    Follow( target );
}

void AGameObject::Follow( AGameObject* go )
{
  Stop(); // DropTargets & Clear Old Waypoint queue

  FollowTarget = go;
  if( FollowTarget )
  {
    if( FollowTarget->Dead ) {
      error( FS( "Trying to follow dead attack target %s", *go->Stats.Name ) );
      FollowTarget = 0;
      return;
    }
    
    FollowTarget->Followers += this;
    Game->flycam->MarkAsFollow( FollowTarget );
  }

  AttackReady = 0; // Do not stop to engage other units.
}

void AGameObject::Attack( AGameObject* go )
{
  Stop(); // DropTargets & Clear Old Waypoint queue

  AttackTarget = go;
  if( AttackTarget )
  {
    if( AttackTarget->Dead ) {
      error( FS( "Trying to attack dead attack target %s", *go->Stats.Name ) );
      AttackTarget = 0;
      return;
    }

    AttackTarget->Attackers += this;
    Game->flycam->MarkAsAttack( AttackTarget );
  }

  AttackReady = 0; // Do not stop to engage other units
}

void AGameObject::SetDestination( FVector d )
{
  for( AShape* flag : NavFlags )
    flag->Destroy();
  NavFlags.clear();
  
  //LOG( "%s moving from %f %f %f to %f %f %f", *Name, Pos.X, Pos.Y, Pos.Z, d.X, d.Y, d.Z );
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
      error( FS( "Waypoint %f %f %f couldn't reach ground",
        Waypoints[i].X, Waypoints[i].Y, Waypoints[i].Z ) );
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
      Waypoints.erase( it );
    }
  }
  
  if( Waypoints.size() >= 2 )
  {
    // Is the distance to the 2nd waypoint closer than that of the 1st waypoint?
    if( FVector::DistSquared( Pos, Waypoints[1] ) < FVector::DistSquared( Pos, Waypoints[0] ) )
    {
      // get rid of point 1
      //info( FS( "Getting rid of point (%f %f %f) in pathway", Waypoints[0].X, Waypoints[0].Y, Waypoints[0].Z ) );
      //Viz( Waypoints[0] );
      pop_front( Waypoints );
    }
  }

  Flags( Waypoints, FLinearColor::Black );
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
  DropTargets(); // Drop Attack & Follow Targets
  StopMoving();  // Clear old waypoint queue
}

void AGameObject::HoldGround()
{
  
}

bool AGameObject::isAllyTo( AGameObject* go )
{
  return team->isAllyTo( go ); // Check with my team
}

bool AGameObject::isEnemyTo( AGameObject* go )
{
  return team->isEnemyTo( go );
}

void AGameObject::DropTargets()
{
  // Tell my old follow target (if any) that I'm no longer following him
  if( FollowTarget )
  {
    //LOG( "%s losing follower %s", *Name, *FollowTarget->Name );
    FollowTarget->LoseFollower( this );
  }

  // If the AttackTarget was already set, tell it loses the old attacker (this).
  if( AttackTarget )
  {
    //LOG( "%s losing attacker %s", *Name, *AttackTarget->Name );
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
    //LOG( "%s doesn't need follow ring", *Name );
    RemoveTagged( this, Game->flycam->FollowTargetName );
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
    //LOG( "%s doesn't need attack ring", *Name );
    RemoveTagged( this, Game->flycam->AttackTargetName );
  }
}

void AGameObject::Stealth()
{
  // Iterate backwards to account for fact that objects removed from array
  // as we iterate. Because we remove objects from the Attackers array
  // as we iterate over it, we CANNOT use a range-based (for( AGameObject* go : Attackers)) style loop
  // (it would cause a runtime error).
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

AGameObject* AGameObject::GetClosestObjectOfType( TSubclassOf<AGameObject> ClassType )
{
  // You can only select within range of unit
  // Go thru all objects on team
  map< float, AGameObject* > m;
  for( AGameObject* go : team->units )
  {
    if( go->IsA( ClassType ) )  skip;
    float d = outerDistance( go );
    m[d] = go;
  }
  if( m.size() )
    return m.begin()->second;

  info( FS( "Could not find an object of type %s", *ClassType->GetName() ) );
  return 0; // NO UNITS In sight
}

void AGameObject::Flags( vector<FVector> points, FLinearColor color )
{
  for( AShape* flag : NavFlags )
    flag->Cleanup();
  NavFlags.clear();

  for( int i = 0; i < points.size(); i++ )
  {
    AShape *flag = Game->Make<AShape>( Game->flycam->WaypointFlagClass, team, points[i] );
    flag->SetColor( color );
    flag->text = FS( "%d", i+1 );
    NavFlags.push_back( flag );
  }
}

void AGameObject::Viz( FVector pt )
{
  Game->flycam->Visualize( Game->flycam->VizClass, pt, 10.f, vizColor );
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
    SetMeshColor( mesh, this, parameterName, color );
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
  Stats.TeamId = team->teamId;
  
  // Grab all meshes with material parameters & set color of each
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>();
  for( UMeshComponent* mesh : meshes )
  {
    SetMeshColor( mesh, this, "TeamColor", team->Color );
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
  DropTargets(); // Remove its attack and follow
  Stealth(); // Attackers and followers stop detecting

  Dead = 1; // updates the blueprint animation and kicks off the death animation in the
  // state machine (usually).

  // Remove from selection.
  if( Game->hud ) Game->hud->Unselect( { this } );
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
    Stealth();
  }

  Destroy();
}

void AGameObject::BeginDestroy()
{
  // For odd-time created objects (esp in PIE) they get put into the team without ever actually
  // being played with, so they don't die properly.
  if( team ) {
    //warning( FS( "Unit %s was removed from team in BeginDestroy()", *Name ) );
    team->RemoveUnit( this );
  }

  if( Attackers.size() || Followers.size() )
  {
    error( FS("There are %d attackers and %d followers", Attackers.size(), Followers.size() ) );
    Stealth();
  }

  // Check if object is selected, only possible game launched/ready
  //info( FS( "%s was destroyed", *Name ) );
  
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

