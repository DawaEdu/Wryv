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

// Sets default values
AGameObject::AGameObject( const FObjectInitializer& PCIP )
{
  LOG( "%s [%s]->AGameObject::AGameObject()", *GetName(), *BaseStats.Name );
  PrimaryActorTick.bCanEverTick = true;

  AttackCooldown = 0;
  Repairing = 0;
  Vel = FVector(0, 0, 0);
  LoadsFromTable = 0;
  FollowTarget = 0;
  AttackTarget = 0;
  NextSpell = Types::NOTHING; // no spell is queued
  OriginalScale = FVector(1,1,1);
  //bounds = PCIP.CreateDefaultSubobject<UCapsuleComponent>( this, "BoundingCapsule" );
  //RootComponent = bounds;

}

void AGameObject::PostInitializeComponents()
{
  LOG( "%s [%s]->AGameObject::PostInitializeComponents()", *GetName(), *BaseStats.Name );
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
  OriginalScale = GetActorScale3D();
  float r=0.f, h=0.f;
  GetComponentsBoundingCylinder( r, h );
  //bounds->SetCapsuleSize( r, h );
  LOG( "%s [%s] AGameObject::OnMapLoaded(), speed=%f", *GetName(), *Stats.Name, Speed );

  if( isBuilding() )  Repairing = 0; // Buildings need an attending peasant to repair
  else  Repairing = 1; // Live units automatically regen

  // Instantiate abilities
  for( int i = 0; i < Stats.Abilities.Num(); i++ )
  {
    // Look up the ability's cooldown time
    Abilities.push_back( Ability( Stats.Abilities[i] ) );
  }

  //Pos = SetOnGround( Pos );
  Dest = Pos;  // set the position to where the thing is.
}

// Called when the game starts or when spawned
void AGameObject::BeginPlay()
{
  Super::BeginPlay();
  LOG( "%s [%s]->AGameObject::BeginPlay()", *GetName(), *BaseStats.Name );
  
  // Put the GameObject on the correct team
  Game->gm->teams[ BaseStats.Team ] -> AddUnit( this );
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
  
  // It will actually scale the child (this) by the accumulated scaling of
  // all parents.  Because I don't want that,  I'll reverse scale the widgets
  //FVector scale = Invert( newParent->OriginalScale );
  //SetActorScale3D( scale );
  //SetActorRelativeScale3D( scale );
  //rc->SetWorldLocation( prc->GetComponentLocation() );
  //rc->SetRelativeLocation( prc->RelativeLocation );
  //rc->SetWorldLocation( prc->GetComponentLocation() );

  //AttachRootComponentToActor( newParent, NAME_None, EAttachLocation::KeepRelativeOffset, 0 );
  //AttachRootComponentToActor( newParent, NAME_None, EAttachLocation::KeepWorldPosition, 0 );
  //rc->AttachTo( prc, NAME_None, EAttachLocation::SnapToTarget );
  //rc->SetRelativeLocation( FVector(0,0,0) );
  //rc->SetWorldLocation( FVector(0,0,0) );
  //AttachRootComponentTo( newParent->GetRootComponent() );
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
  if( !Game->IsReady() )
    return;
  
  // Unset the HUD's selector if its set
  if( isParentOf( Game->hud->selectorAttackTarget ) )
  {
    Game->hud->selectorAttackTarget->SetParent( 0 ); // unparent
  }

  // check if the object is set as a target for any other gameobject
  for( pair<const int32,Team*> p : Game->gm->teams )
  {
    Team* team = p.second;
    for( AGameObject *go : team->units )
    {
      // If the other gameobject has this as an attack target,
      // then null out its attack target.
      //if( go->AttackTarget == this )
      //  go->AttackTarget = 0;
    }
  }
}

FRotator AGameObject::GetRot()
{
  if( RootComponent )
  {
    return RootComponent->GetComponentRotation();
  }
  LOG( "No root component" );
  return FRotator( 0.f );
}

void AGameObject::SetRot( FRotator & ro )
{
  if( RootComponent )
  {
    RootComponent->SetWorldRotation( ro );
  }
  else
  {
    LOG( "No root component" );
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
  // build a building of type specified, if team can afford
  if( team->CanAfford( type ) ) {
    buildQueue.push_back( CooldownCounter( type ) );
    return 1;
  }

  LOG( "%s cannot afford building %s", *Stats.Name, *Game->unitsData[type].Name );
  return 0; // couldn't afford building
}

bool AGameObject::Reached( FVector& v, float dist )
{
  FVector diff = Pos - v;
  diff.Z = 0;
  return diff.Size() < dist;
}

void AGameObject::UpdateDestination()
{
  // How much of the way are we towards our next destination point
  // Check Waypoints. If reached dest, then pop next waypoint.
  if( Reached( Dest, 250.f ) )
  {
    // Pop the next waypoint.
    if( !Waypoints.size() )
      return;

    Dest = Waypoints.front();
    pop_front( Waypoints );
  }

  // Try a bezier curve selection of the destination point...
}

void AGameObject::MoveTowards( float t )
{
  UpdateDestination();

  // MOVEMENT: move the unit @ speed from current position
  // to destination position
  FVector start = Pos;
  Dir = Dest - start;
  float len = Dir.Size();
  //LOG( "%s is %f units away target or Dest", *Stats.Name, len );
  
  static float epsTravel = 1.f;

  // Clamp travel length so that we can't overshoot destination
  if( len > epsTravel )
  {
    Dir = Dir / len; // normalize

    if( !Speed ) {
      // Shouldn't try to travel with 0 speed.
      LOG( "%s had 0 speed", *GetName() );
    }

    // Compute the travel that this object goes
    Vel = Dir*Speed;
    FVector disp = Vel*t;

    // If travel exceeds destination, then jump to dest,
    // so we don't jitter over the final position.
    if( disp.Size() > len )
    {
      Pos = Dest; // we are @ destination.
      Vel = FVector( 0, 0, 0 ); // zero the velocity.
    }
    else
    {
      Pos = start + disp;
    }

    //LOG( "%s @ (%f %f %f)", *GetName(), Pos.X, Pos.Y, Pos.Z );
    // Push UP from the ground plane, using the bounds on the actor.

    FRotator ro = Dir.Rotation();
    ro.Yaw -= 90.f;
    ro.Roll = ro.Pitch = 0;
    SetRot( ro );
  }

  SetOnGround( Pos );
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
    FVector p = Pos, q = AttackTarget->Pos;
    FVector fromTarget = p - q; 
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
        fromTarget /= len; // normalize.
      
      if( !Waypoints.size() )
      {
        FVector d = q + fromTarget*Stats.AttackRange;
        // move the unit only far enough so that it is within the attack range
        Waypoints = Game->flycam->pathfinder->findPath( p, d );
      }
    }
  }
  
  // Move towards the modified ground destination
  MoveTowards( t );
  
  // Flush the computed position to the root component
  RootComponent->SetWorldLocation( Pos );

  // Try and cast spell queued, if any
  // A spell is being cast. Where does it go?
  // `AttackTarget` must be specified, for the spell to be released.
  // Only launch the spell if within range.
  if( NextSpell   &&   AttackTarget )
  {
    // Create the Spell object and send it towards the AttackTarget.
    ASpell* spell = (ASpell*)Game->Make( NextSpell, Pos, Stats.Team );
    
    // If there is no AttackTarget, then there needs to be a dest.
    spell->caster = this;
    spell->AttackTarget = AttackTarget;

    // construct the spell object & launch it @ target.
    // A spell is being cast. Where does it go?
    // `AttackTarget` must be specified, for the spell to be released.
    // Only launch the spell if within range.
    // Create the Spell object and send it towards the AttackTarget.
    // If there is no AttackTarget, then there needs to be a dest.
    NextSpell = NOTHING;
  }
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
      if( g->Stats.Team == Stats.Team )  continue;

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

void AGameObject::SetDestination( FVector d )
{
  if( !Stats.SpeedMax ) {
    LOG( "Warning: Set unit's destination on unit with SpeedMax=0" );
  }

  FVector p = Pos;
  
  // Visualize the start position itself
  //Visualize( p, Game->flycam->Yellow );
  //Visualize( d, Game->flycam->Yellow );
  
  // find the path, then submit list of Waypoints
  Waypoints = Game->flycam->pathfinder->findPath( p, d );
  
  // Fix Waypoints z value so they sit on ground plane
  for( int i = 0; i < Waypoints.size(); i++ )
    Waypoints[i] = SetOnGround( Waypoints[i] );

  if( Waypoints.size() >= 3 )
  {
    // Check the back 2 points, if the 2nd last has a 
    FVector b1 = Waypoints[ Waypoints.size() - 1 ];
    FVector b2 = Waypoints[ Waypoints.size() - 2 ];
    FVector b3 = Waypoints[ Waypoints.size() - 3 ];

    // The two vectors between (b3, b2) and (b2, b1) must not have a large angle between them.
    FVector dir1 = b2 - b3;
    FVector dir2 = b1 - b2;
    dir1.Normalize(), dir2.Normalize();
    const float a = cosf( 30.f );
    float dot = FVector::DotProduct( dir1, dir2 );
    LOG( "DotProduct=%f", dot );
    if( dot < a )
    {
      // Pop the middle one
      Waypoints.erase( Waypoints.begin() + ( Waypoints.size() - 2 ) );
    }
  }

  Game->flycam->Visualize( Waypoints );

  // check for case:
  //
  //                   *
  //     +------------/-
  //     |           /
  //     +----------/---
  //               /
  //         *    / *
  //         |   /  |
  //    -- * x  * * x
  //       |    x |
  //       x      x
  // - that destination that the 2nd point isn't more than 90
  //   degrees away from the 1st point.

  //LOG( LogTemp, Warning,
  //  TEXT("------------------ MAKING A PATHWAY ------------------") );
  //LOG( " (%f %f %f) (%f %f %f) ",
  //  p.X, p.Y, p.Z, d.X, d.Y, d.Z );

  // spawn spheres at each visited waypoint
  // find the pathway for this object using Waypoints set in the level.
  Dest = Waypoints.front();
  pop_front( Waypoints );
  FollowTarget = 0;
  AttackTarget = 0; // Unset the attack target

  //LOG( "%s travelling", *GetName() );
  // use Waypoints set in the level to find a pathway
  // to walk along, if an impassible is encountered along
  // the path.
}

void AGameObject::OnSelected()
{
  
}

FString AGameObject::PrintStats()
{
  // puts the stats into an fstring
  FString stats = FString::Printf( TEXT( "%s\nAttack Damage %d\nArmor %d" ),
    *Stats.Name, Stats.AttackDamage, Stats.Armor );
  return stats;
}

float AGameObject::GetBoundingRadius()
{
  FVector2D size;
  GetComponentsBoundingCylinder( size.X, size.Y, 1 );
  return size.GetMax();
}

void AGameObject::BeginDestroy()
{
	// Remove it from the global collections.
  //LOG( "Destroying %s %s", *Stats.Name, *GetName() );
  
  // During destruction, we have to check the world object exists
  // If the world object exists, then we can get the GameMode. The
  // world object doesn't exist on exit of the editor sometimes.
  
  if( team )
  {
    removeElement( team->units, this );
  }

  removeAsTarget();
  Super::BeginDestroy(); // PUT THIS LAST or the object may become invalid
}

