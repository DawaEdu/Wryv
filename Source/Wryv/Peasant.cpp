#include "Wryv.h"

#include "AI.h"
#include "Building.h"
#include "FlyCam.h"
#include "Goldmine.h"
#include "Pathfinder.h"
#include "Peasant.h"
#include "PlayerControl.h"
#include "Resource.h"
#include "Stone.h"
#include "TheHUD.h"
#include "Townhall.h"
#include "Tree.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

#include "BuildAction.h"
#include "InProgressBuilding.h"
#include "InProgressUnit.h"

APeasant::APeasant( const FObjectInitializer& PCIP ) : AUnit(PCIP)
{
  ResourceCarry = PCIP.CreateDefaultSubobject<USceneComponent>( this, "ResourceCarry1" );
  ResourceCarry->AttachTo( GetRootComponent() );

  GoldPiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "GoldPiece1" );
  GoldPiece->AttachTo( ResourceCarry );
  LumberPiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "LumberPiece1" );
  LumberPiece->AttachTo( ResourceCarry );
  StonePiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "StonePiece1" );
  StonePiece->AttachTo( ResourceCarry );

  GoldPiece->SetVisibility( false );
  LumberPiece->SetVisibility( false );
  StonePiece->SetVisibility( false );

  MinedResources[ AGoldmine::StaticClass() ] = 0;
  MinedResources[ ATree::StaticClass() ] = 0;
  MinedResources[ AStone::StaticClass() ] = 0;

  MinedPieces[ AGoldmine::StaticClass() ] = GoldPiece;
  MinedPieces[ ATree::StaticClass() ] = LumberPiece;
  MinedPieces[ AStone::StaticClass() ] = StonePiece;

  GoldCarryCapacity = LumberCarryCapacity = StoneCarryCapacity = 10;
  Capacities[ AGoldmine::StaticClass() ] = GoldCarryCapacity;
  Capacities[ ATree::StaticClass() ] = LumberCarryCapacity;
  Capacities[ AStone::StaticClass() ] = StoneCarryCapacity;

  RepairTarget = 0;
  Shrugging = 0;
  Mining = 0;
  GatheringRate = 1;

  LastResourcePosition = FVector(0,0,0);
}

void APeasant::PostInitializeComponents()
{
  Super::PostInitializeComponents();

  Capacities[ AGoldmine::StaticClass() ] = GoldCarryCapacity;
  Capacities[ ATree::StaticClass() ] = LumberCarryCapacity;
  Capacities[ AStone::StaticClass() ] = StoneCarryCapacity;

  // Hide the stock pieces.
  MinedPieces[ AGoldmine::StaticClass() ] -> SetVisibility( false );
  MinedPieces[ ATree::StaticClass() ] -> SetVisibility( false );
  MinedPieces[ AStone::StaticClass() ] -> SetVisibility( false );
}

void APeasant::InitIcons()
{
  AUnit::InitIcons();

  for( int i = 0; i < Builds.Num(); i++ )
  {
    if( Builds[i] )
    {
      UBuildAction* action = Construct<UBuildAction>( Builds[i] );
      action->Peasant = this;
      Buildables.Push( action );
    }
  }
}

bool APeasant::UseBuild( int index )
{
  if( index < 0 || index >= Buildables.Num() )
  {
    error( FS( "index %d OOB", index ) );
    return 0;
  }

  Game->flycam->ghost = Game->Make< ABuilding >( Buildables[index]->BuildingType );
  
  return 1;
}

bool APeasant::CancelBuilding( int index )
{
  if( index < 0 || index >= CountersBuildingsQueue.Num() )
  {
    error( FS( "index %d OOB", index ) );
    return 0;
  }

  // Cancels ith building
  UInProgressBuilding* buildingAction = CountersBuildingsQueue[ index ];
  buildingAction->Building->Cancel();
  
  return 1;

}

bool APeasant::Build( UBuildAction* buildAction, FVector pos )
{
  if( team->CanAfford( buildAction->BuildingType ) )
  {
    // Construct the instance of the building
    ABuilding* building = Game->Make<ABuilding>( buildAction->BuildingType, team, pos );

    // Construct the counter & add it to counters for this object
    UInProgressBuilding* buildInProgress = Construct<UInProgressBuilding>( UInProgressBuilding::StaticClass() );
    buildInProgress->Building = building;
    buildInProgress->Peasant = this;
    CountersBuildingsQueue.Push( buildInProgress );

    // Make the building and ask the peasant to join in building it.
    building->PlaceBuilding( this );
    return 1;
  }
  else
  {
    info( FS( "%s cannot afford type %s",
      *Stats.Name, *buildAction->BuildingType->GetName() ) );
    return 0;
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
    if( building->isAllyTo( this ) )
    {
      info( FS( "%s repairing %s", *GetName(), *target->GetName() ) );

      if( !building->PrimaryPeasant ) {
        building->PrimaryPeasant = this;
        info( FS( "%s has been set as the primary builder on building %s", *Stats.Name, *building->Stats.Name ) );
      }
      
      Follow( building ); // The peasant will start repairing it when it gets there.
      RepairTarget = building; // Don't forget to set the repair target! Above line sets attackers and followers to nuls again.
      return;
    }
  }

  AUnit::Target( target );
}

void APeasant::DropTargets()
{
  Super::DropTargets();

  // Drop the RepairTarget.
  if( RepairTarget )
  {
    // tell the repairtarget it lost its repairer
    RepairTarget->LosePeasant( this );
    RepairTarget = 0;
  }
}

void APeasant::Repair( float t )
{
  // If no building was found for repair, don't try and repair null object
  if( RepairTarget )
  {
    if( !in( RepulsionOverlaps, (AGameObject*)RepairTarget ) )
    {
      info( FS( "Peasant %s is too far from building to be repairing it", *Stats.Name ) );
      return;
    }

    float hpRecovered = RepairTarget->Stats.RepairRate * t;
    if( RepairTarget->PrimaryPeasant == this )
    {
      return; // This is the primary builder, so he uses no additional resources when building.
    }
    else
    {
      // If the building's not done yet, repairs cost, but we do a build time add
      if( !RepairTarget->BuildingDone() )
        hpRecovered = RepairTarget->GetHPAdd( t );
    }
    
    // repairs Hp gradually to a building at a fraction of the building's original construction cost.
    // cost the team resources for Repairing this building.
    float goldCost   = RepairTarget->RepairHPFractionCost * hpRecovered * RepairTarget->Stats.GoldCost;
    float lumberCost = RepairTarget->RepairHPFractionCost * hpRecovered * RepairTarget->Stats.LumberCost;
    float stoneCost  = RepairTarget->RepairHPFractionCost * hpRecovered * RepairTarget->Stats.StoneCost;
    
    // Can only repair if won't dip values below zero
    if( team->Gold >= goldCost   &&   team->Lumber >= lumberCost   &&   team->Stone >= stoneCost )
    {
      team->Gold   -= goldCost;
      team->Lumber -= lumberCost;
      team->Stone  -= stoneCost;
      RepairTarget->Hp += hpRecovered;
      info( FS( "Peasant %s has repaired building %s for %f units of hp", *Stats.Name,
        *RepairTarget->Stats.Name, hpRecovered ) );
    }
    else
    {
      // Stop repairing
      Game->hud->Status( "Need more resources to continue repair" );
      DropTargets(); // Now idling.
      return;
    }

    if( RepairTarget->HpFraction() >= 1.f )
    {
      Game->hud->Status( "Building has been fully repaired" );
      DropTargets(); // Now idling.
    }
  }
}

AResource* APeasant::FindAndTargetNewResource( FVector fromPos,
  vector< TSubclassOf<AResource> > types, float searchRadius )
{
  set< TSubclassOf<AGameObject> > acceptable;
  for( TSubclassOf<AResource> r : types )
    acceptable.insert( r );
  vector<AGameObject*> objects = Game->pc->ShapePickExcept( fromPos,
    FCollisionShape::MakeCapsule( searchRadius, Height()/2.f ), acceptable, {} );

  // From return results (which are in order of distance), refilter based on priority ordering in types
  for( int i = 0; i < types.size(); i++ )
  {
    for( int j = 0; j < objects.size(); j++ )
    {
      if( objects[j]->IsA( types[i] ) )
      {
        if( AResource* res = Cast<AResource>( objects[j] ) )
        {
          Target( res );
          //info( FS( "Peasant %s is now mining %s", *Stats.Name, *res->Stats.Name ) );
          return res;
        }
        else
        {
          error( FS( "%s is Type %s but cannot cast to resource", *objects[j]->GetName(), *types[j]->GetName() ) );
        }
      }
    }
  }

  // no resources of types selected were found in radius
  info( FS( "%s found no resources of any type within %f units of %f %f %f", *Stats.Name,
    Stats.SightRange, fromPos.X, fromPos.Y, fromPos.Z ) );
  
  return 0;
}

void APeasant::AttackCycle()
{
  if( AResource *MiningTarget = Cast<AResource>( AttackTarget ) )
  {
    //LOG( "%s mines from %s", *Name, *AttackTarget->Name );
    // can only progress mining if sufficiently close.
    // use distance to object - radius to determine distance you are standing away from object
    // The attackRange of a peasant is used to get the resource gathering range
    if( outerDistance( MiningTarget )   <=   Stats.AttackRange )
    {
      StopMoving();      // Can stop moving, as we mine the resource
      MiningTarget->Jiggle = 1; // This jiggles the animation when the attack cycle
      MiningTarget->Harvest( this );
    }
  }
  else if( RepairTarget )
  {
    info( FS( "Repairing %s, no attack is done", *RepairTarget->Stats.Name ) );
  }
  else
  {
    // Its a regular attack target, pass to AttackCycle to roll the usual attack
    AUnit::AttackCycle();
  }
}

bool APeasant::IsRepairing()
{
  return RepairTarget != NULL   &&   !Speed;
}

bool APeasant::IsCarrying()
{
  for( map< TSubclassOf<AResource>, int32 >::iterator iter = MinedResources.begin();
       iter != MinedResources.end(); ++iter )
    if( iter->second )
      return 1;

  return 0;
}

AGameObject* APeasant::GetBuildingMostInNeedOfRepair( float threshold )
{
  // Is the building worth starting repairs on?
  // We have a heuristic sort of formula:
  //   1. building should be worth starting repairs on
  //   2. we should be able to recover some amount of the building's HP without stopping repairs

  // Get the building most in need of repair
  ABuilding* lowestHpBuilding = 0;
  float lowestHpPerc = 1.f; // 100% Hp
  
  // Could add some logic here to leave higher HP units alone
  for( int i = 0; i < team->units.size(); i++ )
  {
    if( ABuilding* building = Cast<ABuilding>( team->units[ i ] ) )
    {
      float hpPerc = building->HpFraction();
      if( hpPerc < threshold && hpPerc < lowestHpPerc )
      {
        lowestHpBuilding = building;
        lowestHpPerc = hpPerc;
      }
    }
  }

  return lowestHpBuilding;
}

void APeasant::ai( float t )
{
  // already doing something
  if( FollowTarget || AttackTarget ) return;

  // Search for a repair target
  for( AGameObject *go : team->units )
  {
    if( ABuilding* building = Cast<ABuilding>( go ) )
    {
      // Begin repairing building when building's hp is below repair threshold
      if( go->HpFraction() < team->ai.repairFraction )
      {
        Target( go ) ; // repair it
        return;
      }
    }
  }
}

void APeasant::OnResourcesReturned()
{
  if( !Mining )
  {
    warning( FS( "No resource type to return" ) );
    return;
  }

  // What he wants to do is target the same resource type.
  vector< TSubclassOf<AResource> > resType = { Mining }; // We're still mining MiningType unless otherwise specified.

  // Check for reassignment to a new resource type.
  if( team->ai.aiLevel )
  {
    resType = team->GetNeededResourceTypes();
  }

  // go back to get more resources near where we were previously gathering
  // If the mining target ran out of resources, find a new one.
  AResource* res = FindAndTargetNewResource( LastResourcePosition, resType, Stats.SightRange );
  if( res )
  {
    Target( res ) ;
  }
  else
  {
    // Otherwise, he just walks back to the location of the stump, then shrugs.
    GoToGroundPosition( LastResourcePosition );
    ShrugsNextIdle = 1;
  }
}

// This gameobject hitting another
void APeasant::Hit( AGameObject* other )
{
  // if the other building is a townhall, can drop off resources
  if( ATownhall* townhall = Cast<ATownhall>(other) )
  {
    if( townhall->isAllyTo( this ) )
    {
      team->Gold += MinedResources[ AGoldmine::StaticClass() ];
      MinedResources[ AGoldmine::StaticClass() ] = 0;
      MinedPieces[ AGoldmine::StaticClass() ] -> SetVisibility( false );
      
      team->Lumber += MinedResources[ ATree::StaticClass() ];
      MinedResources[ ATree::StaticClass() ] = 0;
      MinedPieces[ ATree::StaticClass() ] -> SetVisibility( false );
      
      team->Stone += MinedResources[ AStone::StaticClass() ];
      MinedResources[ AStone::StaticClass() ] = 0;
      MinedPieces[ AStone::StaticClass() ] -> SetVisibility( false );
      
      Target( 0 ); // unfollow the townhall
      
      // After The resources have been returned
      OnResourcesReturned();
      
      Game->hud->ui->dirty = 1;
    }
  }
}

void APeasant::ReturnResources()
{
  // Attempt to return resources if full-up on any type.
  if( !FollowTarget )
  {
    for( pair< TSubclassOf<AResource>, int32 > p : MinedResources )
    {
      // Check if maxed out on any type
      if( p.second )
      {
        if( p.second >= Capacities[ p.first ] )
        {
          // Return to nearest townhall for dropoff
          AGameObject* returnCenter = GetClosestObjectOfType( ATownhall::StaticClass() );
          if( !returnCenter )
          {
            info( "NO RETURN CENTER" );
          }
          else
          {
            info( FS( "Peasant %s returning to town center %s with %d %s",
              *GetName(), *returnCenter->GetName(), p.second, *p.first->GetName() ) );
            // Check if the resource is exhausted. If so, look for a similar resource
            Follow( returnCenter ); // Set a follow on there (NOT general TARGET() cmd) so that
            // it explicitly sets up as a follow and NOT RepairTarget.
          }
        }
      }
    }
  }
}

void APeasant::AddMined( TSubclassOf<AResource> resourceType, float resAmount )
{
  MinedResources[ resourceType ] += resAmount;
  MinedPieces[ resourceType ] -> SetVisibility( true ); // Now carrying this type
  
  // Check to see if selected and update statspanel if so
  if( Game->hud->Selected.size() && Game->hud->Selected.front() == this )
  {
    Game->hud->ui->dirty = 1;
  }
}

void APeasant::MoveCounters( float t )
{
  for( UInProgressBuilding* ipb : CountersBuildingsQueue )
    ipb->Step( t );
  for( UBuildAction* build : Buildables )
    build->Step( t );
}

void APeasant::Move( float t )
{
  ReturnResources();

  // We repair the building that is currently selected
  Repair( t );
  AUnit::Move( t ); // Calls flush, so we put it last

  if( Idling() )
  {
    if( ShrugsNextIdle )
    {
      Shrugging = 1; // Pass signal to animation blueprint
      ShrugsNextIdle = 0;
    }
  }
}

bool APeasant::Idling()
{
  return AGameObject::Idling() || RepairTarget; // If the repair target is set, i'm not idling.
}

void APeasant::JobDone()
{
  if( JobsDoneSound )
    PlaySound( JobsDoneSound );
  else
    info( FS( "%s: Job done sound not set", *Stats.Name ) );
}

void APeasant::Die()
{
  if( RepairTarget )
  {
    RepairTarget->LosePeasant( this );
  }

  AUnit::Die();
}
