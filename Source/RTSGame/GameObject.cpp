#include "RTSGame.h"
#include "GlobalFunctions.h"
#include "GameObject.h"
#include "MyHUD.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "FlyCam.h"
#include "Spell.h"
#include "Pathfinder.h"
#include "GlobalFunctions.h"

// Sets default values
AGameObject::AGameObject( const FObjectInitializer& PCIP )
{
  PrimaryActorTick.bCanEverTick = true;
  LoadsFromTable = 0;
  followTarget = 0;
  attackTarget = 0;
  NextSpell = Types::NOTHING; // no spell is queued
  attackCooldown = 0;
}

// Called when the game starts or when spawned
void AGameObject::BeginPlay()
{
  UE_LOG( LogTemp, Warning, TEXT("AGameObject::BeginPlay(): %s"), *UnitsData.Name );

  Super::BeginPlay();
  dest = Pos();  // set the position to where the thing is.
  hp = UnitsData.HpMax;
  
  repairing = 1; // Units be default recover some HP each frame
  if( isBuilding() )  repairing = 0; // Buildings need an attending peasant to repair
  // Add the GameObject to the global collection of objects of this type
  // We retrieve the GameMode object here
  //if( ARTSGameGameMode *gm = (ARTSGameGameMode*)GetWorld()->GetAuthGameMode() )
  //  gm->AddUnit( this );

  // AddUnit() adds a Unit to the game world
  // Get the Team corresponding to the Unit.
  team = Game->gm->teams[ UnitsData.Team ];
  team->units.push_back( this );

}

void AGameObject::BeginDestroy()
{
	// Remove it from the global collections.
  //UE_LOG( LogTemp, Warning, TEXT("Destroying %s %s"), *UnitsData.Name, *GetName() );
  
  // During destruction, we have to check the world object exists
  // If the world object exists, then we can get the GameMode. The
  // world object doesn't exist on exit of the editor sometimes.
  
  //UE_LOG( LogTemp, Warning, TEXT("ARTSGameGameMode::RemoveUnit(%s) teamIndex=%d" ),
  //  *go->UnitsData.Name, go->UnitsData.Team );
  if( team )
  {
    remove( team->units, this );
  }

  removeAsTarget();
  Super::BeginDestroy(); // PUT THIS LAST or the object may become invalid
}

float AGameObject::centroidDistance( AGameObject *go )
{
  return FVector::Dist( Pos(), go->Pos() );
}

float AGameObject::outsideDistance( AGameObject *go )
{
  float r1, h1, r2, h2;
  GetComponentsBoundingCylinder( r1, h1, 1 );
  go->GetComponentsBoundingCylinder( r2, h2, 1 );
  float dist = centroidDistance( go );
  dist -= r1 + r2;
  return dist;
}

void AGameObject::SetTeam( int teamId )
{
  UnitsData.Team = teamId;
  team = Game->gm->teams[ teamId ];
}

bool AGameObject::ally( AGameObject* go )
{
  return go->team->alliance != Alliance::Neutral   &&
         team == go->team;
}

bool AGameObject::enemy( AGameObject* go )
{
  return go->team->alliance != Alliance::Neutral   &&
         team != go->team;
}

void AGameObject::removeAsTarget()
{
  if( !Game->IsReady() )
    return;
  
  if( Game->myhud->SelectedObject )
  {
    if( Game->myhud->SelectedObject == this )
    {
      // this was the last clicked object, so remove the selector
      Game->myhud->selector->SetActorLocation( FVector(0.f) );
    }
    else if( Game->myhud->SelectedObject->attackTarget == this )
    {
      Game->myhud->selectorAttackTarget->SetActorLocation( FVector(0.f) );
    }
  }

  // check if the object is set as a target for any other gameobject
  for( int i = 0; i < Game->gm->teams.size(); i++ )
  {
    Team* team = Game->gm->teams[i];
    for( int j = 0; j < team->units.size(); j++ )
    {
      // Check all game objects, and make sure none
      AGameObject* go = team->units[j];
      //UE_LOG( LogTemp, Warning, TEXT("Other unit %s"), *go->UnitsData.Name );
      if( this == go->attackTarget )
      {
        UE_LOG( LogTemp, Warning, TEXT("Attack target was %s"), *UnitsData.Name );
        go->attackTarget = 0;
        // If the last clicked object was the gameObject,
        // then unselect it in the hud
        if( this == Game->myhud->SelectedObject )
          Game->myhud->selectorAttackTarget->SetActorLocation( FVector(0.f) );
      }
    }
  }
}

FVector AGameObject::Pos()
{
  if( !RootComponent )
  {
    UE_LOG( LogTemp, Warning, TEXT("No root component") );
    return FVector(0.f);
  }

  return RootComponent->GetComponentLocation();
}

void AGameObject::SetPos(const FVector& pos)
{
  if( !RootComponent )
  {
    UE_LOG( LogTemp, Warning, TEXT("No root component") );
    return;
  }

  RootComponent->SetWorldLocation( pos );
}  

FRotator AGameObject::GetRot()
{
  if( RootComponent )
  {
    return RootComponent->GetComponentRotation();
  }
  return FRotator( 0.f );
}

void AGameObject::SetRot( FRotator & ro )
{
  if( RootComponent )
  {
    RootComponent->SetWorldRotation( ro );
  }
}

// Cast a spell at a target, either at owner's target or
// at a specific vector location
void AGameObject::CastSpell( Types type, AGameObject *target )
{
  // The NextSpell to be cast is (whatever spell was requested)
  NextSpell = type;
  attackTarget = target;
}

bool AGameObject::Reached( FVector& v, float dist )
{
  FVector diff = Pos() - v;
  diff.Z = 0;
  return diff.Size() < dist;
}

void AGameObject::UpdateDestination()
{
  // How much of the way are we towards our next destination point
  // Check waypoints. If reached dest, then pop next waypoint.
  if( Reached( dest, 250.f ) )
  {
    // Pop the next waypoint.
    if( !waypoints.size() )
      return;

    dest = waypoints.front();
    pop_front( waypoints );
  }

  // Try a bezier curve selection of the destination point...
}

void AGameObject::MoveTowards( float t )
{
  UpdateDestination();

  // MOVEMENT:
  // move the unit @ speed from current position
  // to destination position
  FVector start = Pos();
  FVector dir = dest - start;
  float len = dir.Size();
  //UE_LOG( LogTemp, Warning, TEXT("%s is %f units away target or dest"), *UnitsData.Name, len );
  
  static float epsTravel = 1.f;
  // Clamp travel length so that we can't overshoot destination
  if( len > epsTravel )
  {
    FVector NDir = dir / len; // normalize

    // Compute the travel that this object goes
    FVector travel = NDir*UnitsData.Speed*t;

    // If travel exceeds destination, then jump to dest.
    if( travel.Size() > len )  SetPos( dest );
    else  SetPos( start + travel );

    FRotator ro = dir.Rotation();
    ro.Yaw -= 90.f;
    ro.Roll = ro.Pitch = 0;
    SetRot( ro );
  }
}

void AGameObject::SetTarget( AGameObject* go )
{
  attackTarget = go;
  SetDestination( go->Pos() );
}

void AGameObject::StopMoving()
{
  waypoints.clear(); // clear the waypoints
  dest = Pos(); // You are at your destination
}

void AGameObject::Stop()
{
  StopMoving();
  attackTarget = 0;
}

// Called every frame
void AGameObject::Tick( float t )
{
	Super::Tick( t );

  // Don't tick when the game's not ready
  // this happens due to async load, esp between loading levels.
  if( !Game->IsReady() )
  {
    return;
  }

  // Recover HP at stock recovery rate
  if( repairing ) {
    hp += UnitsData.RepairHPFractionCost*t;
  }

  // Call the ai for this object type
  ai( t );
  
  // dest is modified by the attackTarget.
  // Move towards the destination as long as we're not in attackRange
  if( attackTarget )
  {
    // If there is an attackTarget, then the destination
    // position is set at (AttackRange) units back from the attackTarget.
    // only need to move towards attack target if out of range.
    FVector p = Pos(), q = attackTarget->Pos();
    FVector fromTarget = p - q; 
    float len = fromTarget.Size();

    // if out of range, need to move closer
    // Melee weapons (and spell bodies) have an attackRange of 0,
    // since they just fly into the target.
    if( len < UnitsData.AttackRange )
    {
      Stop();
      dest = Pos(); // don't move then
    }
    else if( len >= UnitsData.AttackRange )
    {
      if( len ) // Don't normalize when zero length.
        fromTarget /= len; // normalize.
      
      if( !waypoints.size() )
      {
        FVector d = q + fromTarget*UnitsData.AttackRange;
        // move the unit only far enough so that it is within the attack range
        waypoints = Game->flycam->pathfinder->findPath( p, d );
      }
    }

    //UE_LOG( LogTemp, Warning, TEXT("%s (%f %f %f) is moving towards %s ")
    //  TEXT( "(%f %f %f), dest=(%f %f %f) %f units away, range=%d"),
    //  *UnitsData.Name, p.X, p.Y, p.Z, *attackTarget->UnitsData.Name,
    //  q.X, q.Y, q.Z, dest.X, dest.Y, dest.Z,
    //  len, UnitsData.AttackRange );
  }
  
  // Move towards the modified ground destination
  MoveTowards( t );
  
  // Try and cast spell queued, if any
  // A spell is being cast. Where does it go?
  // `attackTarget` must be specified, for the spell to be released.
  // Only launch the spell if within range.
  if( NextSpell   &&   attackTarget )
  {
    // Create the Spell object and send it towards the attackTarget.
    ASpell* spell = (ASpell*)Game->Make( NextSpell, Pos(), UnitsData.Team );
    
    // If there is no attackTarget, then there needs to be a dest.
    spell->caster = this;
    spell->attackTarget = attackTarget;

    // construct the spell object & launch it @ target.
    // A spell is being cast. Where does it go?
    // `attackTarget` must be specified, for the spell to be released.
    // Only launch the spell if within range.
    // Create the Spell object and send it towards the attackTarget.
    // If there is no attackTarget, then there needs to be a dest.
    NextSpell = NOTHING;
  }
}

void AGameObject::ai( float t )
{
  // Base GameObject doesn't have AI for it.
}

void AGameObject::fight( float t )
{
  // If we have an attackTarget and we are close enough to it,
  // and the cooldown is over, we attack it
  if( attackTarget   &&   attackCooldown <= 0.f   &&
      outsideDistance( attackTarget ) < UnitsData.AttackRange )
  {
    // calculate the damage done to the target by the attack
    float damage = UnitsData.AttackDamage - attackTarget->UnitsData.Armor;

    // reset the attackCooldown to being full amount
    attackCooldown = UnitsData.AttackCooldown;
  }

  // cooldown a little bit
  attackCooldown -= t;
}

float AGameObject::hpPercent()
{
  if( ! UnitsData.HpMax ) // if max hp not set, just return hp it has
    return hp;
  return hp / UnitsData.HpMax;
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
      if( g->UnitsData.Team == UnitsData.Team )  continue;

      float d = FVector::Dist( g->Pos(), Pos() );
      if( d < UnitsData.SightRange )
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
    if( g   &&   g->UnitsData.Type == type )
    {
      float dist = FVector::Dist( Pos(), g->Pos() );
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
  FCollisionQueryParams fcqp( "dest trace", true );
  fcqp.AddIgnoredActor( this );
  FCollisionObjectQueryParams fcoqp;
  return GetWorld()->LineTraceSingleByObjectType( hit, Pos(), p, fcoqp, fcqp );
}

void AGameObject::SetDestination( FVector d )
{
  FVector p = Pos();
  
  // Visualize the start position itself
  //Visualize( p, Game->flycam->Yellow );
  //Visualize( d, Game->flycam->Yellow );
  
  // find the path, then submit list of waypoints
  waypoints = Game->flycam->pathfinder->findPath( p, d );
  
  // Fix waypoints z value so they sit on ground plane
  //FBox box = Game->flycam->floor->GetComponentsBoundingBox();
  //for( int i = 0; i < waypoints.size(); i++ )
  //  waypoints[i].Z = box.Max.Z;

  if( waypoints.size() >= 3 )
  {
    // Check the back 2 points, if the 2nd last has a 
    FVector b1 = waypoints[ waypoints.size() - 1 ];
    FVector b2 = waypoints[ waypoints.size() - 2 ];
    FVector b3 = waypoints[ waypoints.size() - 3 ];

    // The two vectors between (b3, b2) and (b2, b1) must not have a large angle between them.
    FVector dir1 = b2 - b3;
    FVector dir2 = b1 - b2;
    dir1.Normalize(), dir2.Normalize();
    const float a = cosf( 30.f );
    float dot = FVector::DotProduct( dir1, dir2 );
    UE_LOG( LogTemp, Warning, TEXT("DotProduct=%f"), dot );
    if( dot < a )
    {
      // Pop the middle one
      waypoints.erase( waypoints.begin() + ( waypoints.size() - 2 ) );
    }
  }

  Game->flycam->Visualize( waypoints );

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

  //UE_LOG( LogTemp, Warning,
  //  TEXT("------------------ MAKING A PATHWAY ------------------") );
  //UE_LOG( LogTemp, Warning, TEXT(" (%f %f %f) (%f %f %f) "),
  //  p.X, p.Y, p.Z, d.X, d.Y, d.Z );

  // spawn spheres at each visited waypoint
  // find the pathway for this object using waypoints set in the level.
  dest = waypoints.front();
  pop_front( waypoints );
  followTarget = 0;
  attackTarget = 0; // Unset the attack target

  //UE_LOG( LogTemp, Warning, TEXT("%s travelling"), *GetName() );
  // use waypoints set in the level to find a pathway
  // to walk along, if an impassible is encountered along
  // the path.
}

// Populate the spawnqueue
void AGameObject::RefreshBuildingQueue()
{
  StackPanel* building = Game->myhud->ui->building;

  // Clear existing widgets
  building->Clear();

  // Things that are spawning.
  for( int i = 0; i < spawnQueue.size(); i++ )
  {
    SpawningObject so = spawnQueue[i];
    
    // Program-in removal logic
    ImageWidget *img = new ImageWidget( Game->myhud->widgets[ so.Type ].Icon );
    img->OnClicked = [this,i](FVector2D mouse){
      // Remove this entry from the spawnQueue
      // then refresh the spawn queue
      UE_LOG( LogTemp, Warning, TEXT( "Removing elt %d from spawnqueue" ), i );
      remove( spawnQueue, i );
      RefreshBuildingQueue();
      return 0;
    };
    img->OnHover = [](FVector2D mouse){
      return 0;
    };
    building->StackRight( img );
  }
}

void AGameObject::OnSelected()
{
  // Clear the items palette
  Game->myhud->ui->itemBelt->Clear();

  Game->myhud->ui->itemBelt->SetNumSlots( 0, 0 );

  // when a gameobject is selected, the picture/icon is updated
  //Game->myhud->selectedIcon->Icon = Widget.Icon;
  RefreshBuildingQueue();

  // track object's attacktarget
  Game->myhud->SetAttackTargetSelector( attackTarget );
}

FString AGameObject::PrintStats()
{
  // puts the stats into an fstring
  FString stats = FString::Printf( TEXT( "%s\nAttack Damage %d\nArmor %d" ),
    *UnitsData.Name, UnitsData.AttackDamage, UnitsData.Armor );
  return stats;
}

float AGameObject::GetBoundingRadius()
{
  FVector2D size;
  GetComponentsBoundingCylinder( size.X, size.Y, 1 );
  return size.GetMax();
}

