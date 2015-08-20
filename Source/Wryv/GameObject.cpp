#include "Wryv.h"
#include "GlobalFunctions.h"
#include "GameObject.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "FlyCam.h"
#include "Spell.h"
#include "Pathfinder.h"
#include "GlobalFunctions.h"
#include "PlayerControl.h"
#include "Widget3D.h"

const float AGameObject::WaypointAngleTolerance = 30.f; // 
const float AGameObject::WaypointReachedToleranceDistance = 250.f; // The distance to consider waypoint as "reached"

AGameObject* AGameObject::Nothing = 0;

// Sets default values
AGameObject::AGameObject( const FObjectInitializer& PCIP )
{
  //LOG( "%s [%s]->AGameObject::AGameObject()", *GetName(), *BaseStats.Name );
  PrimaryActorTick.bCanEverTick = true;
  AttackCooldown = 0;
  Repairing = 0;
  team = 0;
  Pos = Vel = FVector(0, 0, 0);
  FollowTarget = AttackTarget = 0;
  NextAction = Types::NOTHING; // no spell is queued
}

void AGameObject::PostInitializeComponents()
{
  //LOG( "%s [%s]->AGameObject::PostInitializeComponents()", *GetName(), *BaseStats.Name );
  Super::PostInitializeComponents();
  //if( !Game->IsReady() )
  //{
  //  warning( FS( "%s: GameInstance not initialized", *Stats.Name ) );
  //  Pos = Rand();
  //  return;
  //}

  if( RootComponent )
  {
    // Initialize position, but put object on the ground
    Pos = RootComponent->GetComponentLocation();
  }

  UpdateStats( 0.f );
  Hp = Stats.HpMax;
  Speed = Stats.SpeedMax;

  if( isBuilding() )  Repairing = 0; // Buildings need an attending peasant to repair
  else  Repairing = 1; // Live units automatically regen
  
  Dest = Pos;  // set the position to where the thing is.
}

// Called when the game starts or when spawned
void AGameObject::BeginPlay()
{
  Super::BeginPlay();
  //LOG( "%s [%s]->AGameObject::BeginPlay()", *GetName(), *BaseStats.Name );
  SetTeam( Stats.TeamId );

  // Instantiate abilities
  for( int i = 0; i < Stats.Abilities.Num(); i++ )
    Abilities.push_back( CooldownCounter( Stats.Abilities[i] ) );
}

void AGameObject::OnMapLoaded()
{
  
}

AGameObject* AGameObject::SetParent( AGameObject* newParent )
{
  USceneComponent *pc = 0;
  if( newParent )  pc = newParent->GetRootComponent();
  SetOwner( newParent );
  // Set the world position to being that of parent, then keep world position on attachment
  GetRootComponent()->SetWorldScale3D( FVector(1,1,1) ); // reset the scale off
  GetRootComponent()->SetRelativeScale3D( FVector(1,1,1) ); 
  GetRootComponent()->SnapTo( pc );
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

AGameObject* AGameObject::MakeChild( Types type )
{
  AGameObject* child = Game->Make<AGameObject>( type );
  AddChild( child );
  return child;
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
  float r1, h1, r2, h2;
  GetComponentsBoundingCylinder( r1, h1, 1 );
  go->GetComponentsBoundingCylinder( r2, h2, 1 );
  float dist = centroidDistance( go );
  dist -= r1 + r2;
  return dist;
}

void AGameObject::Action()
{
  if( NextAction != NOTHING )
  {
    if( IsSpell( NextAction ) )
    {
      if( Game->GetData( NextAction ).GroundAttack || // Ground attack casts regardless
        (!Game->GetData( NextAction ).GroundAttack && AttackTarget ) ) // Non-ground with target
      {
        ASpell* spell = Game->Make<ASpell>( NextAction, Pos, Stats.TeamId );
        spell->caster = this;
        spell->Attack( AttackTarget );
        spell->AttackTargetOffset = AttackTargetOffset;
        NextAction = NOTHING;
      }
    }
  }
}

// Cast a spell at a target, either at owner's target or
// at a specific vector location
void AGameObject::Action( Types type, AGameObject *target )
{
  // The NextAction to be cast by this actor is (whatever spell was requested)
  NextAction = type;
  Attack( target );
}

// Cast spell with a target location
void AGameObject::Action( Types type, FVector where )
{
  NextAction = type;
  Attack( 0 );
  AttackTargetOffset = where;
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

  // tick builds
  for( int i = BuildQueueCounters.size() - 1; i >= 0; i-- ) {
    BuildQueueCounters[i].Time += t;
    // update the viz of the build queue counter, ith clock
    if( BuildQueueCounters[i].Done() )
    {
      // Remove it and consider ith building complete. Place @ unoccupied position around building.
      FVector buildPos = Pos + GetActorForwardVector() * GetBoundingRadius();
      
      Game->Make( BuildQueueCounters[i].Type, buildPos, team->teamId );

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
    NextAction = type;
  }
  else if( IsBuilding( type ) )
  {
    info( FS( "Building a %s", *GetTypesName( type ) ));
    
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

bool AGameObject::isAttackTargetWithinRange()
{
  if( AttackTarget )  return distanceToAttackTarget() < Stats.AttackRange;
  return 0;
}

float AGameObject::distanceToAttackTarget()
{
  if( !AttackTarget )  return FLT_MAX;
  return centroidDistance( AttackTarget );
}

float AGameObject::hpPercent()
{
  if( ! Stats.HpMax ) {
    error( FS( "HpMax not set for %s", *Stats.Name ) );
    return 1.f;
  }
  else  return Hp / Stats.HpMax;  // if max hp not set, just return hp it has
}

float AGameObject::speedPercent()
{
  if( ! Stats.SpeedMax )
  {
    error( FS( "SpeedMax not set for %s", *Stats.Name ) );
    return 1.f;
  }
  return Speed / Stats.SpeedMax;
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
  // Old follow target loses a follower
  if( FollowTarget )
    FollowTarget->LoseFollower( this );
  FollowTarget = go;
  if( go )
  {
    go->Followers.push_back( this );
    SetDestination( go->Pos );
    Game->hud->SelectAsFollow( go );
  }
}

void AGameObject::StopFollowing()
{
  // Notify follow target that I'm no longer following him.
  if( FollowTarget )
    FollowTarget->LoseFollower( this );
  FollowTarget = 0;
}

void AGameObject::LoseFollower( AGameObject* formerFollower )
{
  if( !formerFollower ) error( "Cannot lose null follower" );
  formerFollower->Follow( 0 );
  removeElement( Followers, formerFollower );
  // if I lost all followers, update the hud
  if( !Followers.size() )
    Game->hud->UnselectAsFollow( this );
}

void AGameObject::LoseAllFollowers()
{
  for( AGameObject* fol : Followers )
    fol->Follow( 0 );
  Followers.clear();
}

void AGameObject::Attack( AGameObject* go )
{
  if( AttackTarget )
    AttackTarget->LoseAttacker( this );
  AttackTarget = go;
  if( go )
  {
    go->Attackers.push_back( this );
    SetDestination( go->Pos );
    Game->hud->SelectAsAttack( go );
  }
}

void AGameObject::StopAttacking()
{
  if( AttackTarget )
    AttackTarget->LoseAttacker( this );
  AttackTarget = 0;
}

void AGameObject::LoseAttacker( AGameObject* formerAttacker )
{
  if( !formerAttacker ) error( "Cannot lose null follower" );
  removeElement( Attackers, formerAttacker );
  // If there are no more attackers, unselect in ui
  if( !Attackers.size() )
    Game->hud->UnselectAsAttack( this );
}

void AGameObject::LoseAllAttackers()
{
  // Tell all attackers I'm no longer available for attack.
  for( AGameObject* att : Attackers )
    att->Attack( 0 );
  Attackers.clear();
}

void AGameObject::SetRot( const FRotator & ro )
{
  if( RootComponent ) RootComponent->SetWorldRotation( ro );
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

void AGameObject::Walk( float t )
{
  CheckWaypoint();

  Dir = Dest - Pos;
  float len = Dir.Size();
  static float epsTravel = 1.f;

  // Clamp travel length so that we can't overshoot destination
  if( len > epsTravel )
  {
    Dir = Dir / len; // normalize
    if( !Speed )  error( FS("%s had 0 speed", *GetName()) );
    
    Vel = Dir*Speed;
    FVector disp = Vel*t;

    // If travel exceeds destination, then jump to dest,
    // so we don't jitter over the final position.
    if( disp.Size() < len )  Pos += disp;
    else
    {
      Pos = Dest; // we are @ destination.
      Vel = FVector( 0, 0, 0 );
    }

    // Push UP from the ground plane, using the bounds on the actor.
    SetRot( Dir.Rotation() );
  }

  Game->flycam->SetOnGround( Pos + Game->flycam->floorBox.Max.Z*1.01f );
}

void AGameObject::SetDestination( FVector d )
{
  LOG( "%s moving from %f %f %f to %f %f %f", *Stats.Name,
    Pos.X, Pos.Y, Pos.Z, d.X, d.Y, d.Z );
  if( !Stats.SpeedMax ) {
    error( "Warning: Set unit's destination on unit with SpeedMax=0" );
  }

  // Make sure the destination is grounded
  d = Game->flycam->SetOnGround( d );
  
  // find the path, then submit list of Waypoints
  Waypoints = Game->flycam->pathfinder->findPath( Pos, d );
  
  // Fix Waypoints z value so they sit on ground plane
  for( int i = 0; i < Waypoints.size(); i++ )
  {
    Waypoints[i].Z = Game->flycam->floorBox.Max.Z*1.01f; // position ABOVE the ground plane.
    //!!BUG: Sometimes a point doesn't hit the ground plane, and will end up at the upper limits
    // of the bounding box.
    Waypoints[i] = Game->flycam->SetOnGround( Waypoints[i] ); // Then set on ground.
  }

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
      Waypoints.erase( --(--(Waypoints.end())) );
      //Game->flycam->Visualize( *it, 64.f, FLinearColor::Black );
    }
  }

  // Visualize the pathway
  //Game->flycam->ClearViz();
  //Game->flycam->Visualize( Waypoints, 32.f, FLinearColor( 0, 0.8f, 0, 1.f ),
  //  FLinearColor( 0.8f, 0.8f, 0, 1.f ) );
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
  Follow( 0 );
  Attack( 0 );
}

void AGameObject::Move( float t )
{
  // Update & Cache Unit's stats this frame.
  UpdateStats( t );
  
  // Call the ai for this object type
  ai( t );
  
  // recompute path
  if( FollowTarget )  SetDestination( FollowTarget->Pos );
  if( AttackTarget )  SetDestination( AttackTarget->Pos );

  Walk( t );   // Walk towards destination
  
  DoAttack( t );
  
  Action();

  // Flush the computed position to the root component
  RootComponent->SetWorldLocation( Pos );
}

void AGameObject::ai( float t )
{
  // Base GameObject doesn't have AI for it.
}

void AGameObject::DoAttack( float t )
{
  // If we have an AttackTarget and we are close enough to it,
  // and the cooldown is over, we attack it
  if( AttackTarget   &&   AttackCooldown <= 0.f   &&
      outsideDistance( AttackTarget ) < Stats.AttackRange )
  {
    // calculate the damage done to the target by the attack
    float damage = Stats.AttackDamage - AttackTarget->Stats.Armor;

    // reset the AttackCooldown to being full amount
    AttackCooldown = Stats.AttackCooldown;
  }

  // cooldown a little bit
  AttackCooldown -= t;
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
	for( pair<int32,Team*> p : Game->gm->teams )
  {
    Team* otherTeam = p.second;
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

bool AGameObject::LOS( FVector p )
{
  FHitResult hit;
  FCollisionQueryParams fcqp( "Dest trace", true );
  fcqp.AddIgnoredActor( this );
  FCollisionObjectQueryParams fcoqp;
  return GetWorld()->LineTraceSingleByObjectType( hit, Pos, p, fcoqp, fcqp );
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

void AGameObject::SetTeam( int32 teamId )
{
  // If the old team this unit was on is set, remove it from that team.
  if( team ) { team->RemoveUnit( this ); }
  // change team.
  BaseStats.TeamId = Stats.TeamId = teamId;
  team = Game->gm->GetTeam( teamId );
  team->AddUnit( this );
}

void AGameObject::Die()
{
  // Don't call DESTROY for a few frames.

  // Spawn explosion animation (particle emitter).
  MakeChild( (Types)(EXPLOSION1 + randInt(0,3)) );

  // Turn off collisions.

}

void AGameObject::BeginDestroy()
{
	// Remove it from the global collections.
  if( Game->IsReady() )
  {
    LoseAllFollowers();
    LoseAllAttackers();
    if( team )  team->RemoveUnit( this );
  }
  Super::BeginDestroy(); // PUT THIS LAST or the object may become invalid
}

