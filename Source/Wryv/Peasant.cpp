#include "Wryv.h"
#include "Peasant.h"
#include "Resource.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "FlyCam.h"
#include "Pathfinder.h"
#include "AI.h"
#include "Building.h"
#include "TheHUD.h"
#include "PlayerControl.h"

APeasant::APeasant( const FObjectInitializer& PCIP ) : AUnit(PCIP)
{
  ResourceCarry = PCIP.CreateDefaultSubobject<USceneComponent>( this, "ResourceCarry1" );
  ResourceCarry->AttachTo( RootComponent );

  GoldPiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "GoldPiece1" );
  GoldPiece->AttachTo( ResourceCarry );
  LumberPiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "LumberPiece1" );
  LumberPiece->AttachTo( ResourceCarry );
  StonePiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "StonePiece1" );
  StonePiece->AttachTo( ResourceCarry );

  GoldPiece->SetVisibility( false );
  LumberPiece->SetVisibility( false );
  StonePiece->SetVisibility( false );

  MinedResources[ Types::RESGOLD ] = 0;
  MinedResources[ Types::RESLUMBER ] = 0;
  MinedResources[ Types::RESSTONE ] = 0;

  MinedPieces[ Types::RESGOLD ] = GoldPiece;
  MinedPieces[ Types::RESLUMBER ] = LumberPiece;
  MinedPieces[ Types::RESSTONE ] = StonePiece;

  Building = 0;
  Carrying = 0;
  Repairing = 0;
  Shrugging = 0;
  Mining = NOTHING;

  //RepairTarget = 0;
  LastResourcePosition = FVector(0,0,0);
}

void APeasant::PostInitializeComponents()
{
  Super::PostInitializeComponents();

  Capacities[ Types::RESGOLD ] = GoldCarryCapacity;
  Capacities[ Types::RESLUMBER ] = LumberCarryCapacity;
  Capacities[ Types::RESSTONE ] = StoneCarryCapacity;
}

void APeasant::Build( Types type, FVector pos )
{
  if( team->CanAfford( type ) )
  {
    // Make the building and ask the peasant to join in building it.
    ABuilding* building = Game->Make<ABuilding>( type, team, pos );
    building->PlaceBuilding( this );
  }
}

void APeasant::Target( AGameObject* target )
{
  if( AResource *resource = Cast<AResource>( target ) )
  {
    // "attack" the resource to mine it.
    LastResourcePosition = resource->Pos;
    Attack( resource );
    return;
  }

  else if( ABuilding *building = Cast<ABuilding>( target ) )
  {
    if( building->isAllyTo( this ) ) {
      info( FS( "%s repairing %s", *GetName(), *target->GetName() ) );
      Repairing = 1;
      return;
    }
  }

  AUnit::Target( target );
}

void APeasant::Repair( float t )
{
  // If no building was found for repair, don't try and repair null object
  if( FollowTarget && FollowTarget->isBuilding() )
  {
    AGameObject* RepairTarget = FollowTarget;

    // repairs Hp gradually to a building at a fraction of the building's original construction cost.
    // cost the team resources for Repairing this building.
    float hpRecovered = RepairTarget->Stats.RepairRate * t; // HP/s*time
    float goldCost    = RepairTarget->Stats.RepairHPFractionCost * hpRecovered * RepairTarget->Stats.GoldCost;
    float lumberCost  = RepairTarget->Stats.RepairHPFractionCost * hpRecovered * RepairTarget->Stats.LumberCost;
    float stoneCost   = RepairTarget->Stats.RepairHPFractionCost * hpRecovered * RepairTarget->Stats.StoneCost;

    // Can only repair if won't dip values below zero
    if( team->Gold >= goldCost   &&   team->Lumber >= lumberCost   &&   team->Stone >= stoneCost )
    {
      team->Gold   -= goldCost;
      team->Lumber -= lumberCost;
      team->Stone  -= stoneCost;
      RepairTarget->Hp   += hpRecovered;
    }
    else
    {
      // Stop repairing
      Game->hud->Status( "Need more resources to continue repair" );
    }
  }
}

AResource* APeasant::FindNewResource( FVector fromPos, Types type, float searchRadius )
{
  vector<AGameObject*> objects = Game->pc->ShapePickExcept( fromPos,
    FCollisionShape::MakeCapsule( Radius(), 100.f ), { type }, {} );
  if( objects.size() )
  {
    if( AResource* res = Cast<AResource>( objects[0] ) )
      return res;
    else
      error( FS( "%s is not a Resource object", *objects[0]->GetName() ) );
  }
  
  return 0;
}

void APeasant::AttackCycle()
{
  if( AResource *MiningTarget = Cast<AResource>( AttackTarget ) )
  {
    //LOG( "%s mines from %s", *Stats.Name, *AttackTarget->Stats.Name );
    // can only progress mining if sufficiently close.
    // use distance to object - radius to determine distance you are standing away from object
    // The attackRange of a peasant is used to get the resource gathering range
    float distance = outerDistance( MiningTarget );
    if( distance   <=   Stats.AttackRange )
    {
      StopMoving();      // Can stop moving, as we mine the resource
      MiningTarget->Jiggle = 1; // This jiggles the animation when the attack cycle
      MiningTarget->Harvest( this );
    }
  }
  else
  {
    // Its a regular attack target, pass to AttackCycle to roll the usual attack
    AGameObject::AttackCycle();
  }
}

AGameObject* APeasant::GetBuildingMostInNeedOfRepair( float threshold )
{
  // Is the building worth starting repairs on?
  // We have a heuristic sort of formula:
  //   1. building should be worth starting repairs on
  //   2. we should be able to recover some amount of the building's HP without stopping repairs

  // Get the building most in need of repair
  AGameObject* lowestHpUnit = 0;
  float lowestHpPerc = 1.f; // 100% Hp
  
  // Could add some logic here to leave higher HP units alone
  for( int i = 0; i < team->units.size(); i++ )
  {
    AGameObject *g = team->units[ i ];
    if( g->isBuilding() )
    {
      float hpPerc = g->HpFraction();
      if( hpPerc < threshold && hpPerc < lowestHpPerc )
      {
        lowestHpUnit = g;
        lowestHpPerc = hpPerc;
      }
    }
  }
  return lowestHpUnit;
}

void APeasant::ai( float t )
{
  // already doing something
  if( FollowTarget || AttackTarget ) return;

  // Search for a repair target
  for( AGameObject *go : team->units )
  {
    if( go->isBuilding() && go->HpFraction() < .5f )
    {
      Follow( go ) ; // repair it
      return;
    }
  }
}

void APeasant::OnResourcesReturned()
{
  // Resources returned. If it is AI controlled, then
  // we may harvest a new type of resources here.
  if( team->ai.level )
  {
    Types neededResType = team->GetNeededResourceType();

    // CHANGE resource type being mined.
    if( neededResType != Mining )
    {
      // Try and find something of this new needed type in the vicinity.
      if( AResource* res = FindNewResource( Pos, neededResType, Stats.SightRange*3.f ) )
      {
        Target( res );
      }
      else
      {
        // go where the resources were at least and seek further cmd
        GoToGroundPosition( LastResourcePosition );
      }
    }
  }

  // go back to get more resources near where we were previously gathering
  // If the mining target ran out of resources, find a new one.
  AResource* res = FindNewResource( LastResourcePosition, Mining, Stats.SightRange );
  if( res )  Target( res ) ;
  else {
    GoToGroundPosition( LastResourcePosition );
  }
}

// This gameobject hitting another
void APeasant::Hit( AGameObject* other )
{
  // if the other building is a townhall, can drop off resources
  if( other->Stats.Type == Types::BLDGTOWNHALL )
  {
    team->Gold += MinedResources[ Types::RESGOLD ];
    MinedResources[ Types::RESGOLD ] = 0;
    team->Lumber += MinedResources[ Types::RESLUMBER ];
    MinedResources[ Types::RESLUMBER ] = 0;
    team->Stone += MinedResources[ Types::RESSTONE ];
    MinedResources[ Types::RESSTONE ] = 0;
    Follow( 0 ); // unfollow the townhall

    // The resources have been returned
    OnResourcesReturned();
  }
}

void APeasant::ReturnResources()
{
  // Attempt to return resources if full-up on any type.
  if( !FollowTarget )
  {
    for( pair< Types, int32 > p : MinedResources )
    {
      if( p.second > 0 )
      {
        Carrying = 1; // Apply carrying animation.
        MinedPieces[ p.first ] -> SetVisibility( true );
        if( p.second >= Capacities[ p.first ] )
        {
          // Return to nearest townhall for dropoff
          AGameObject* returnCenter = GetClosestObjectOfType( Types::BLDGTOWNHALL );
          if( !returnCenter )
          {
            info( "NO RETURN CENTER" );
          }
          else
          {
            //info( FS( "Peasant %s returning to town center %s with %d %s",
            //  *GetName(), *returnCenter->GetName(), p.second, *GetTypesName( p.first ) ) );
            // Check if the resource is exhausted. If so, look for a similar resource
            Follow( returnCenter ); // Keep the attacktarget (miningtarget) set.
          }
        }
      }
      else
      {
        MinedPieces[ p.first ] -> SetVisibility( false );
      }
    }
  }
}

void APeasant::Move( float t )
{
  Carrying = 0;
  ReturnResources();

  // We repair the building that is currently selected
  if( FollowTarget )  Repair( t );
  AUnit::Move( t ); // Calls flush, so we put it last

  if( Idling() )
  {
    if( Mining ) // Trying to mine something but couldn't find it.
    {
      // go back to where the resource was and shrug
      Shrugging = 1;
      Mining = NOTHING;
    }
  }
}

void APeasant::JobDone()
{
  if( JobsDoneSound )
    PlaySound( JobsDoneSound );
  else
    info( FS( "%s: Job done sound not set", *Stats.Name ) );
}

