#include "Wryv.h"
#include "Peasant.h"
#include "Resource.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "FlyCam.h"
#include "Pathfinder.h"
#include "AI.h"
#include "Building.h"

APeasant::APeasant( const FObjectInitializer& PCIP ) : AUnit(PCIP)
{
  MiningTime = 0.f;
  building = 0;
  repair = 0;
}

void APeasant::BeginPlay()
{
  Super::BeginPlay();
}

AGameObject* APeasant::Build( Types type, FVector pos )
{
  // Builds the building @ pos.
  ABuilding* b = Cast<ABuilding>( Game->Make( type, pos, team->teamId ) );

  // Try and place the building can be placed @ position
  if( PlaceBuilding( b, pos ) )  return b;
  else  return NULL;
}

bool APeasant::PlaceBuilding( ABuilding* b, FVector pos )
{
  // Check if building is placeable here.
  // Uses the graph to determine placeability, and puts nodes to blocked
  // if makes region impassible.
  b->SetPos( pos );
  FBox box = b->GetComponentsBoundingBox();
  for( int i = 0; i < Game->flycam->pathfinder->nodes.size(); i++ )
  {
    GraphNode* g = Game->flycam->pathfinder->nodes[i];
    if( Intersects( box, g->point ) )
    {
      if( g->terrain == Impassible )
      {
        // can't place building here
        return false;
      }
      else if( g->terrain == Passible )
      {
        g->terrain = Impassible;
        Game->flycam->pathfinder->updateGraphConnections( g->index );
      }
    }
  }

  building = b;
  return true;
}

AGameObject* APeasant::PlaceBuildingAtRandomLocation( Types type )
{
  // What we want to do here is if there is a building to build,
  // we start the unit out towards that building's placement location.
  
  // Try to choose a location for the building to be placed.
  // What we do here is basically search the local space for
  // a space large enough for the building.
  // start at the town hall and jitter randomly until we find a placement location
  UClass* uc = Game->unitsData[ type ].uClass;

  // Create the building (for size/dimensions @ placement attempts)
  building = Cast<ABuilding>( GetWorld()->SpawnActor( uc ) );
  
  // Get the dimensions of the building to know how to shift it
  // between placement attempts
  FBox newBldgExtents = building->GetComponentsBoundingBox();

  // Search for building location
  team = Game->gm->teams[ UnitsData.Team ];
  AGameObject* townhall = team->GetFirstOfType( Types::BLDGTOWNHALL );
  FVector loc;
  
  // Place it around the townhall
  if( townhall )  loc = townhall->pos;
  else  loc = team->GetTownCentroid();

  // Jitter the location. Get the map size.
  FVector floorpos, floorext, pos, ext;
  FBox extents = Game->flycam->floor->GetComponentsBoundingBox();
  extents.GetCenterAndExtents( floorpos, floorext );
  extents = townhall->GetComponentsBoundingBox();
  extents.GetCenterAndExtents( pos, ext );
    
  // Search in location surrounding townhall or loc.
  FVector floorll = floorpos - floorext/2.f;
  FVector floortr = floorpos + floorext/2.f;
  FVector ll = pos - ext/2.f;
  FVector tr = pos + ext/2.f;

  // Say try a 10x10 square around the townhall or town centroid.
  // if the town gets larger or there is no room, then don't build the building
  // (AI limits).
  bool placed = 0;
  for( int i = -10; i < 10 && !placed; i++ )
  {
    for( int j = -10; j < 10 && !placed; j++ )
    {
      FVector p = loc;
      p.X += newBldgExtents.GetExtent().X * i;
      p.Y += newBldgExtents.GetExtent().Y * j;
      building->SetPos( p );

      // If the building can be placed here, we place it.
      if( !Game->flycam->intersectsAny( building ) )
      {
        placed = 1;
      }
    }
  }

  PlaceBuilding( building, loc );
  
  LOG( "Placement failed");
  return building;
}

void APeasant::Build( float t )
{
  if( building )
  {
    // proceed with building current building, if exists
    // if multiple peasants are building the same building, then
    // the building progresses faster
    building->buildProgress += t;
    if( building->buildProgress > building->UnitsData.BuildTime )
    {
      // building is complete.
      building = 0;
      LOG( "Building complete");
    }
  }
}

void APeasant::Repair( float t )
{
  if( !repair )
  {
    // No unit has been selected for repair. Check to see if any buildings need repair.
    repair = GetBuildingMostInNeedOfRepair( team->DamageRepairThreshold );
  }

  // If no building was found for repair, don't try and repair null object
  if( repair )
  {
    // repairs hp gradually to a building at a fraction of the building's original construction cost.
    // cost the team resources for repairing this building.
    float hpRecovered = repair->UnitsData.RepairRate * t; // HP/s*time
    float goldCost   = repair->UnitsData.RepairHPFractionCost * hpRecovered * repair->UnitsData.GoldCost;
    float lumberCost = repair->UnitsData.RepairHPFractionCost * hpRecovered * repair->UnitsData.LumberCost;
    float stoneCost  = repair->UnitsData.RepairHPFractionCost * hpRecovered * repair->UnitsData.StoneCost;

    // Can only repair if won't dip values below zero
    if( team->Gold >= goldCost   &&   team->Lumber >= lumberCost   &&   team->Stone >= stoneCost )
    {
      team->Gold   -= goldCost;
      team->Lumber -= lumberCost;
      team->Stone  -= stoneCost;
      repair->hp   += hpRecovered;
    }

    // if it has completed repair, deselect for repair
    if( repair->hpPercent() == 1.f ) {
      repair = 0;
    }
  }
}

void APeasant::Mine( float t )
{
  // When a peasant is "attacking" a tree or goldmine,
  // it is actually harvesting from it
  if( mining )
  {
    // can only progress mining if sufficiently close.
    // use distance to object - radius to determine distance you are standing away from object
    // The attackRange of a peasant is used to get the resource gathering range
    if( outsideDistance( attackTarget )   <=   UnitsData.AttackRange )
    {
      StopMoving();      // Can stop moving, as we mine the resource
      MiningTime -= t;   // Progress mining.

      // The peasant has to be mining the resource
      // for a certain amount of time
      // 
      // The amount of time to mine the resource is in
      // the ResourceTypes map
      if( MiningTime < 0 )
      {
        // Just mined it. Get the resource.
        switch( mining->UnitsData.Type )
        {
          case RESTREE:  team->Lumber += mining->Multiplier;  break;
          case RESGOLDMINE:  team->Gold += mining->Multiplier;  break;
          case RESSTONE:  team->Stone += mining->Multiplier;  break;
        }

        // We've mined.
        mining->Amount--;
        
        // Reset mining time. TimeLength is a polymorphic property.
        MiningTime = Game->unitsData[ mining->UnitsData.Type ].TimeLength;
        
        if( !mining->Amount )
        {
          mining->Destroy();
          attackTarget = 0;
        }
      }
    }
  }
  
  /////////////////////////
  // Check if I have to change what I'm mining, or if I have to find a new
  // resource to mine.

  // Not building a building, so try and mine resources
  // select type to mine
  // 
  // Select a resource type to mine, based on need
  Types neededResType = team->GetNeededResourceType();

  // if the resource type i'm mining changed..
  if( !attackTarget   ||   neededResType != attackTarget->UnitsData.Type )
  {
    // may have changed, but only change mining type after
    // successful mining operation of this type
    // Try and find an object of type resType in the level
    attackTarget = GetClosestObjectOfType( neededResType );

    // Reset mining time remaining
    if( attackTarget ) {
      MiningTime = attackTarget->UnitsData.TimeLength; // polymorphic property
    }
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
  float lowestHpPerc = 1.f; // 100% hp
  
  // Could add some logic here to leave higher HP units alone
  for( int i = 0; i < team->units.size(); i++ )
  {
    AGameObject *g = team->units[ i ];
    if( g->isBuilding() )
    {
      float hpPerc = g->hpPercent();
      if( hpPerc < threshold && hpPerc < lowestHpPerc )
      {
        lowestHpUnit = g;
        lowestHpPerc = hpPerc;
      }
    }
  }
  return lowestHpUnit;
}

void APeasant::SetTarget( AGameObject* go )
{
  // Depending on the type of object GO is, we set it as either building, mining, attack or repair target
  if( AResource* res = Cast<AResource>( go ) )
  {
    mining = res; // starting to mine this resource
  }
  else if( go->isBuilding() )
  {
    building = Cast<ABuilding>( go );
  }
}

void APeasant::ai( float t )
{
  // Assign a function to the object by what the AI wants the peasant to do
}

void APeasant::Move( float t )
{
  // Moves the Peasant object 
  // Work on the current building.
  Build( t );
  
  // We repair the building that is currently selected
  if( !building )
  {
    Repair( t );
  }
  
  if( !building && !repair )
  {
    // Start mining, if we aren't repairing or building something
    Mine( t );
  }
}



