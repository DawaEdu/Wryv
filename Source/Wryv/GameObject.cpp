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

// Sets default values
AGameObject::AGameObject( const FObjectInitializer& PCIP )
{
  //LOG( "%s [%s]->AGameObject::AGameObject()", *GetName(), *BaseStats.Name );
  PrimaryActorTick.bCanEverTick = true;
  AttackCooldown = 0;
  Repairing = 0;

  Pos = Vel = FVector(0, 0, 0);
  FollowTarget = AttackTarget = 0;
  NextSpell = Types::NOTHING; // no spell is queued
  UE_LOG( K, Warning, TEXT( "OK" ) );
}

void AGameObject::PostInitializeComponents()
{
  //LOG( "%s [%s]->AGameObject::PostInitializeComponents()", *GetName(), *BaseStats.Name );
  Super::PostInitializeComponents();
  if( RootComponent )
  {
    // Initialize position, but put object on the ground
    Pos = RootComponent->GetComponentLocation();
  }

  UpdateStats();
  Stats = BaseStats;
  Hp = Stats.HpMax;
  Speed = Stats.SpeedMax;

  if( isBuilding() )  Repairing = 0; // Buildings need an attending peasant to repair
  else  Repairing = 1; // Live units automatically regen

  // Instantiate abilities
  for( int i = 0; i < Stats.Abilities.Num(); i++ )
    Abilities.push_back( Ability( Stats.Abilities[i] ) );
  
  Pos = SetOnGround( Pos );
  Dest = Pos;  // set the position to where the thing is.
}

// Called when the game starts or when spawned
void AGameObject::BeginPlay()
{
  Super::BeginPlay();
  //LOG( "%s [%s]->AGameObject::BeginPlay()", *GetName(), *BaseStats.Name );
  SetTeam( Stats.TeamId );
}

void AGameObject::OnMapLoaded()
{

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

bool AGameObject::isAttackTargetWithinRange()
{
  if( AttackTarget )
  {
    return distanceToAttackTarget() < Stats.AttackRange;
  }

  return 0;
}

float AGameObject::distanceToAttackTarget()
{
  if( !AttackTarget )  return FLT_MAX;
  return centroidDistance( AttackTarget );
}

float AGameObject::hpPercent()
{
  if( ! Stats.HpMax )  return Hp;
  else  return Hp / Stats.HpMax;  // if max hp not set, just return hp it has
}

float AGameObject::speedPercent()
{
  return Speed / Stats.SpeedMax;
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
  AWidget3D* child = Game->Make<AWidget3D>( type );
  AddChild( child );
  return child;
}

bool AGameObject::isAlly( AGameObject* go )
{
  return team == go->team   &&   go->team->alliance != Alliance::Neutral ;
}

bool AGameObject::isEnemy( AGameObject* go )
{
  return team != go->team   &&   go->team->alliance != Alliance::Neutral;
}

void AGameObject::removeAsTarget()
{
  Game->hud->UnselectAsTarget( this );

  // Remove as target from all teams
  for( pair<const int32,Team*> p : Game->gm->teams )
  {
    Team* team = p.second;
    for( AGameObject *go : team->units )
      if( go->AttackTarget == this )
        go->AttackTarget = 0;
  }
}

void AGameObject::SetRot( const FRotator & ro )
{
  if( RootComponent ) RootComponent->SetWorldRotation( ro );
  else error( "No root component" );
}

void AGameObject::CastSpell()
{
  if( NextSpell != NOTHING )
  {
    ASpell* spell = Game->Make<ASpell>( NextSpell, Pos, Stats.TeamId );
    spell->caster = this;
    spell->AttackTarget = AttackTarget;
    spell->AttackTargetOffset = AttackTargetOffset;
    NextSpell = NOTHING;
  }
}

// Cast a spell at a target, either at owner's target or
// at a specific vector location
void AGameObject::CastSpell( Types type, AGameObject *target )
{
  // The NextSpell to be cast is (whatever spell was requested)
  NextSpell = type;
  AttackTarget = target;
}

// Cast spell with a target location
void AGameObject::CastSpell( Types type, FVector where )
{
  NextSpell = type;
  AttackTargetOffset = where;
}

void AGameObject::ApplyEffect( FUnitsDataRow item )
{
  // TimeLength, dataSet
  LOG( "Applying %s for %f seconds", *item.Name, item.TimeLength );
  // Don't do anything for the Nothing item
  if( !IsItem( item.Type ) )
  {
    LOG( "%s NOT AN ITEM", *item.Name );
  }
  else
  {
    BonusTraits.push_back( PowerUpTimeOut( item.TimeLength, item ) );
  }
}

void AGameObject::UseAbility( Ability& ability )
{
  if( ability.Done() ) // ability is ready.
  {
    // You can use the ability now
    Game->unitsData[ ability.Type ];

    // Reset refresh time.
    ability.Reset(); // ability not ready now
  }
}

void AGameObject::UseAbility( Ability& ability, AGameObject *target )
{
}

void AGameObject::UpdateStats()
{
  Stats = BaseStats;
  for( int i = 0; i < BonusTraits.size(); i++ )
    Stats += BonusTraits[i].traits;
}

bool AGameObject::Build( Types type )
{
  if( team->CanAfford( type ) ) {
    BuildQueueCounters.push_back( CooldownCounter( type ) );
    return 1;
  }

  info( FS( "%s cannot afford building %s", *Stats.Name, *Game->unitsData[type].Name ) );
  return 0;
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

  SetOnGround( Pos );
}

void AGameObject::SetDestination( FVector d )
{
  if( !Stats.SpeedMax ) {
    LOG( "Warning: Set unit's destination on unit with SpeedMax=0" );
  }

  // Visualize the start position itself
  //Visualize( p, Game->flycam->Yellow );
  //Visualize( d, Game->flycam->Yellow );
  
  // find the path, then submit list of Waypoints
  Waypoints = Game->flycam->pathfinder->findPath( Pos, d );
  
  // Fix Waypoints z value so they sit on ground plane
  for( int i = 0; i < Waypoints.size(); i++ )
    Waypoints[i] = SetOnGround( Waypoints[i] );

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
    error( FS( "DotProduct=%f", dot ) );
    if( dot < a )
    {
      // Pop the middle one
      Waypoints.erase( Waypoints.begin() + ( Waypoints.size() - 2 ) );
    }
  }

  Game->flycam->Visualize( Waypoints );

  Dest = Waypoints.front();
  pop_front( Waypoints );
  FollowTarget = 0;
  AttackTarget = 0; // Unset the attack target
}

void AGameObject::SetTarget( AGameObject* go )
{
  AttackTarget = go;
  SetDestination( go->Pos );
}

void AGameObject::StopMoving()
{
  Waypoints.clear(); // clear the Waypoints
  Dest = Pos; // You are at your destination
}

void AGameObject::Stop()
{
  StopMoving();
  AttackTarget = 0;
}

FVector AGameObject::SetOnGround( FVector v )
{
  FVector floorPos = Game->flycam->getHitFloor( v );
  // account for bounds half-height
  //Pos.Z = bounds->GetScaledCapsuleHalfHeight() + floorPos.Z;//with bounds
  v.Z = floorPos.Z;
  return v;
}

// BASE.
void AGameObject::Move( float t )
{
  // Update & Cache Unit's stats this frame.
  UpdateStats();

  // Recover HP at stock recovery rate
  if( Repairing ) {
    Hp += Stats.RepairHPFractionCost*t;
  }

  // Call the ai for this object type
  ai( t );
  
  // dest is modified by the AttackTarget.
  // Move towards the destination as long as we're not in attackRange
  if( AttackTarget )
  {
    // If there is an AttackTarget, then the destination
    // position is set at (AttackRange) units back from the AttackTarget.
    // only need to move towards attack target if out of range.
    FVector fromTarget = Pos - AttackTarget->Pos; 
    float len = fromTarget.Size();

    // if out of range, need to move closer
    // Melee weapons (and spell bodies) have an attackRange of 0,
    // since they just fly into the target.
    if( len < Stats.AttackRange )
    {
      Stop();
      Dest = Pos; // don't move then
    }
    else if( len >= Stats.AttackRange )
    {
      if( len ) // Don't normalize when zero length.
        fromTarget /= len; // norm
      
      if( !Waypoints.size() )
      {
        FVector d = AttackTarget->Pos + fromTarget*Stats.AttackRange;
        // move the unit only far enough so that it is within the attack range
        Waypoints = Game->flycam->pathfinder->findPath( Pos, d );
      }
    }
  }
  
  Walk( t );   // Walk towards destination
  RootComponent->SetWorldLocation( Pos );  // Flush the computed position to the root component

  // Cast spell if any
  CastSpell();
}

void AGameObject::ai( float t )
{
  // Base GameObject doesn't have AI for it.
}

void AGameObject::fight( float t )
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
  TTransArray<AActor*> &array = GetLevel()->Actors;
	for( int i = 0; i < array.Num(); i++ )
  {
    if( AGameObject *g = Cast<AGameObject>( array[i] ) )
    {
      // Cannot Target unit on same team (also prevents targetting self)
      if( g->Stats.TeamId == Stats.TeamId )  continue;

      float d = FVector::Dist( g->Pos, Pos );
      if( d < Stats.SightRange )
      {
        distances[ d ] = g;
      }
    }
  }

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

void AGameObject::BeginDestroy()
{
	// Remove it from the global collections.
  if( Game->IsReady() )
  {
    if( team )  team->RemoveUnit( this );
    removeAsTarget();
  }
  Super::BeginDestroy(); // PUT THIS LAST or the object may become invalid
}

