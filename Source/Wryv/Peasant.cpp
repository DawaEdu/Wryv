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

APeasant::APeasant( const FObjectInitializer& PCIP ) : AUnit(PCIP)
{
  MiningTime = 0.f;
}

void APeasant::BeginPlay()
{
  Super::BeginPlay();
}

void APeasant::Target( AGameObject* target )
{
  AResource *resource = Cast<AResource>( target );
  if( resource )
  {
    // "attack" the resource to mine it.
    Attack( resource );
    MiningTime = resource->Stats.TimeLength;
  }
  else
  {
    AUnit::Target( target );
  }
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
  }
}

void APeasant::Mine( float t )
{
  if( AResource *MiningTarget = Cast<AResource>( AttackTarget ) )
  {
    // can only progress mining if sufficiently close.
    // use distance to object - radius to determine distance you are standing away from object
    // The attackRange of a peasant is used to get the resource gathering range
    float distance = outerDistance( MiningTarget );
    if( distance   <=   Stats.AttackRange )
    {
      //LOG( "Distance to target is %f %f units", distance, Stats.AttackRange );
      StopMoving();      // Can stop moving, as we mine the resource
      MiningTime -= t;   // Progress mining.

      MiningTarget->Jiggle = 1; // This jiggles the animation when the attack cycle

      // The peasant has to be mining the resource
      // for a certain amount of time
      // 
      // The amount of time to mine the resource is in
      // the ResourceTypes map
      if( MiningTime < 0 )
      {
        MiningTarget->Harvest( this );

      }
    }
  }
  else
  {
    // Its a regular attack target, pass to AttackCycle to roll the usual attack
    AttackCycle();
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
  // Search for a repair target

  /////////////////////////
  // Check resource type I'm mining is correct resource type to mine.
  // Select a resource type to mine, based on need
  Types neededResType = team->GetNeededResourceType();
  
  AResource* MiningTarget = Cast<AResource>( AttackTarget );

  // if the resource type i'm mining changed..
  if( !MiningTarget   ||   neededResType != MiningTarget->Stats.Type )
  {
    // may have changed, but only change mining type after
    // successful mining operation of this type
    // Try and find an object of type resType in the level
    MiningTarget = Cast<AResource>( GetClosestObjectOfType( neededResType ) );
    Attack( MiningTarget );
    // Reset mining time remaining
    MiningTime = MiningTarget->Stats.TimeLength;
  }
}

void APeasant::Move( float t )
{
  // We repair the building that is currently selected
  if( FollowTarget )  Repair( t );
  
  // The attacktarget designates what to mine
  if( AttackTarget )  Mine( t );

  AUnit::Move( t ); // Calls flush, so we put it last
}



