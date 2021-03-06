#include "Wryv.h"

#include "AI/AI.h"
#include "GameObjects/Buildings/Building.h"
#include "UE4/Flycam.h"
#include "GameObjects/Things/Resources/Goldmine.h"
#include "AI/Pathfinder.h"
#include "GameObjects/Units/Peasant.h"
#include "UE4/PlayerControl.h"
#include "GameObjects/Things/Resources/Resource.h"
#include "GameObjects/Things/Resources/Stone.h"
#include "UE4/TheHUD.h"
#include "GameObjects/Buildings/Townhall.h"
#include "GameObjects/Things/Resources/Tree.h"
#include "UE4/WryvGameInstance.h"
#include "UE4/WryvGameMode.h"

#include "UI/UICommand/Command/UIBuildActionCommand.h"
#include "UI/UICommand/Counters/UIInProgressBuildingCounter.h"
#include "UI/UICommand/Counters/UIInProgressUnitCounter.h"

APeasant::APeasant( const FObjectInitializer& PCIP ) : AUnit(PCIP)
{
  ResourceCarry = PCIP.CreateDefaultSubobject<USceneComponent>( this, TEXT( "ResourceCarry" ) );
  ResourceCarry->AttachTo( GetRootComponent() );

  GoldPiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, TEXT( "GoldPiece" ) );
  GoldPiece->AttachTo( ResourceCarry );
  LumberPiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, TEXT( "LumberPiece" ) );
  LumberPiece->AttachTo( ResourceCarry );
  StonePiece = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, TEXT( "StonePiece" ) );
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

  CarryCapacities.Gold = CarryCapacities.Lumber = CarryCapacities.Stone = 10;
  Capacities[ AGoldmine::StaticClass() ] = CarryCapacities.Gold;
  Capacities[ ATree::StaticClass() ] = CarryCapacities.Lumber;
  Capacities[ AStone::StaticClass() ] = CarryCapacities.Stone;

  RepairTarget = 0;
  Shrugging = 0;
  Mining = 0;
  GatheringRate = 1;

  LastResourcePosition = Zero;
}

void APeasant::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  
  Capacities[ AGoldmine::StaticClass() ] = CarryCapacities.Gold;
  Capacities[ ATree::StaticClass() ] = CarryCapacities.Lumber;
  Capacities[ AStone::StaticClass() ] = CarryCapacities.Stone;

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
      UUIBuildActionCommand* action = Construct<UUIBuildActionCommand>( Builds[i] );
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

  TSubclassOf<ABuilding> BuildingClass = Buildables[index]->BuildingClass;
  // Check if the player can afford this building class
  if( team->CanAfford( BuildingClass ) )
  {
    // Setup the ghost to being BuildingClass we like
    Game->flycam->ghost = Game->Make< ABuilding >( BuildingClass );
    return 1;
  }
  else
  {
    info( FS( "Team %s cannot afford to build a %s", *team->Name, *BuildingClass->GetName() ) );
    return 0;
  }
}

bool APeasant::CancelBuilding( int index )
{
  if( index < 0 || index >= CountersBuildingsQueue.Num() )
  {
    error( FS( "index %d OOB", index ) );
    return 0;
  }

  // Cancels ith building
  UUIInProgressBuilding* buildingAction = CountersBuildingsQueue[ index ];
  buildingAction->Building->Cancel();
  
  return 1;
}

// From the UI button, there's PROPOSEBUILD so you can't
// go thru with UI click when can't afford the building..
bool APeasant::Build( TSubclassOf<ABuilding> BuildingClass, FVector position )
{
  if( !team->CanAfford( BuildingClass ) )
  {
    //err-out
    error( FS( "Team %s cannot afford BuildingClass %s", *team->Name, *BuildingClass->GetName() ) );
    return 0;
  }
  // Check against techtree if this building class is allowed for this team now.
  ///else if( !team->TECHTREECHECK(BuildingClass) )
  else
  {
    // Actually construct the building.
    ABuilding* building = Game->Make<ABuilding>( BuildingClass, team, position );

    // Construct the counter & add it to counters for this object
    UUIInProgressBuilding* buildInProgress = 
      Construct<UUIInProgressBuilding>( UUIInProgressBuilding::StaticClass() );
    buildInProgress->Building = building;
    buildInProgress->Peasant = this;
    CountersBuildingsQueue.Push( buildInProgress );

    // Make the building and ask the peasant to join in building it.
    building->PlaceBuilding( this );
    return 1;
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
    FCost tickRepairCost = RepairTarget->Stats.Cost * RepairTarget->RepairHPFractionCost * hpRecovered;
    
    // Can only repair if won't dip values below zero
    if( team->Resources >= tickRepairCost )
    {
      team->Resources -= tickRepairCost;
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
    
  FCollisionShape selShape = FCollisionShape::MakeCapsule( searchRadius, Height()/2.f );
  vector<AGameObject*> objects = Game->pc->ShapePick( fromPos,
    selShape, acceptable, {} );

  // From return results (which are in order of distance), refilter based on priority ordering in types
  for( int i = 0; i < types.size(); i++ ) // Priority order by type. eg [Tree,Stone,Gold].
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
  // Goldmines require the peasant to be inside, while other resources have
  // the peasant outside
  if( AGoldmine *goldmine = Cast<AGoldmine>( AttackTarget ) )
  {
    goldmine->Harvest( this );
  }
  else if( AResource *MiningTarget = Cast<AResource>( AttackTarget ) )
  {
    //LOG( "%s mines from %s", *Name, *AttackTarget->Name );
    // can only progress mining if sufficiently close.
    // use distance to object - radius to determine distance you are standing away from object
    // The attackRange of a peasant is used to get the resource gathering range
    if( outerDistance( MiningTarget )   <=   Stats.AttackRange )
    {
      // Close enough to harvest tree/stone
      StopMoving();      // Can stop moving, as we mine the resource
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
      team->Resources.Gold += MinedResources[ AGoldmine::StaticClass() ];
      MinedResources[ AGoldmine::StaticClass() ] = 0;
      MinedPieces[ AGoldmine::StaticClass() ] -> SetVisibility( false );
      
      team->Resources.Lumber += MinedResources[ ATree::StaticClass() ];
      MinedResources[ ATree::StaticClass() ] = 0;
      MinedPieces[ ATree::StaticClass() ] -> SetVisibility( false );
      
      team->Resources.Stone += MinedResources[ AStone::StaticClass() ];
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
          if( returnCenter )
          {
            //info( FS( "Peasant %s returning to town center %s with %d %s",
            //  *GetName(), *returnCenter->GetName(), p.second, *p.first->GetName() ) );
            // Check if the resource is exhausted. If so, look for a similar resource
            Follow( returnCenter ); // Set a follow on there (NOT general TARGET() cmd) so that
            // it explicitly sets up as a follow and NOT RepairTarget.
          }
          else // !returnCenter
          {
            info( FS( "NO RETURN CENTER" ) );
          }
        }
      }
    }
  }
  else if( ATownhall* townhall = Cast<ATownhall>( FollowTarget ) )
  {
    if( in( RepulsionOverlaps, FollowTarget ) )
    {
      // We may be following the return center
      // if the other building is a townhall, can drop off resources
      if( townhall->isAllyTo( this ) )
      {
        team->Resources.Gold += MinedResources[ AGoldmine::StaticClass() ];
        MinedResources[ AGoldmine::StaticClass() ] = 0;
        MinedPieces[ AGoldmine::StaticClass() ] -> SetVisibility( false );
        
        team->Resources.Lumber += MinedResources[ ATree::StaticClass() ];
        MinedResources[ ATree::StaticClass() ] = 0;
        MinedPieces[ ATree::StaticClass() ] -> SetVisibility( false );
        
        team->Resources.Stone += MinedResources[ AStone::StaticClass() ];
        MinedResources[ AStone::StaticClass() ] = 0;
        MinedPieces[ AStone::StaticClass() ] -> SetVisibility( false );
        
        Target( 0 ); // unfollow the townhall
        
        // After the resources have been returned
        OnResourcesReturned();
        
        Game->hud->ui->dirty = 1;
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
  for( int i = (int)CountersBuildingsQueue.Num() - 1; i >= 0; i-- )
    CountersBuildingsQueue[i]->Step( t );
  for( int i = (int)Buildables.Num() - 1; i >= 0; i-- )
    Buildables[i]->Step( t );
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
