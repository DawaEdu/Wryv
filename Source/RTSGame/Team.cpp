#include "RTSGame.h"
#include "Team.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "Peasant.h"
#include "CombatUnit.h"
#include "FlyCam.h"

Team::Team()
{
  Defaults();
  teamId = 0;
  Name = "Team";
}

Team::Team( int iTeamId, FString str )
{
  Defaults();
  teamId = iTeamId;
  Name = str;
}

void Team::Defaults()
{
  Gold = 500;
  Lumber = 500;
  Stone = 250;
  DamageRepairThreshold = 2.f/3.f;
  alliance = Alliance::Neutral;
}

APeasant* Team::GetNextAvailablePeasant()
{
  for( int i = 0; i < units.size(); i++ )
    if( APeasant *p = Cast<APeasant>( units[i] ) )
      if( !p->isBusy() )
        return p;
  return 0;
}

vector<ACombatUnit*> Team::GetWarriors()
{
  vector<ACombatUnit*> combatUnits;
  for( int i = 0; i < units.size(); i++ )
    if( ACombatUnit* cu = Cast<ACombatUnit>( units[i] ) )
      combatUnits.push_back( cu );
  return combatUnits;
}

void Team::Construct( Types buildingType )
{
  // search for a peasant to construct the building
  APeasant *p = GetNextAvailablePeasant();
  if( !p )
  {
    UE_LOG( LogTemp, Warning, TEXT("Cannot construct %s, no available peasants!"), *GetEnumName(buildingType) );
    return;
  }
  else
  {
    // Peasants build their building if they are supposed to build it,
    // or harvest materials if they are otherwise idle.
    // Try and build it with this peasant, if it can be afforded.
    if( CanAfford( buildingType ) )
    {
      // Create the building and set it as a target for the peasant
      p->PlaceBuildingAtRandomLocation( buildingType );
    }
  }
}

void Team::AddUnit( AGameObject *go )
{
  units.push_back( go );
}

void Team::RemoveUnit( AGameObject *go )
{
  remove( units, go );
}

bool Team::CanAfford( Types type )
{
  FUnitsDataRow row = Game->unitsData[ type ];
  return Gold >= row.GoldCost && Lumber >= row.LumberCost && Stone  >= row.StoneCost;
}

bool Team::Spend( Types type )
{
  FUnitsDataRow row = Game->unitsData[ type ];
  if( CanAfford( type ) )
  {
    Gold   -= row.GoldCost;
    Lumber -= row.LumberCost;
    Stone  -= row.StoneCost;
    return 1;
  }
  return 0;
}


// Usage of food by units in the game.
int Team::computeFoodUsage()
{
  /// Units use food
  int foodUse = 0;
  for( int i = 0; i < units.size(); i++ )
    foodUse += units[i]->UnitsData.FoodUsed;
  return foodUse;
}

// Count by #farms instantiated
int Team::computeFoodSupply()
{
  int foodSupply = 0;
  // The food supply is from Farm objects
  for( int i = 0; i < units.size(); i++ )
    foodSupply += units[i]->UnitsData.FoodProvided;
  return foodSupply;
}

int Team::GetNumberOf( Types type )
{
  int count = 0;
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->UnitsData.Type == type )
      count++;
  return count;
}

AGameObject* Team::GetFirstOfType( Types type )
{
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->UnitsData.Type == type )
      return units[i];
  return 0; // not found
}

// Get the centroid of the town (avg position of all buildings)
FVector Team::GetTownCentroid()
{
  FVector v;
  for( int i = 0; i < units.size(); i++ )
    if( IsBuilding( units[i]->UnitsData.Type ) )
      v += units[i]->Pos();
  return v;
}

bool Team::isNeedsFood()
{
  float ratio = (float)computeFoodUsage() / computeFoodSupply();
  return ratio > ai.foodPercentage ;
}

Types Team::GetNeededResourceType()
{
  return ai.GetNeededResourceType( *this );
}

void Team::runAI( float t )
{
  ai.timeSinceLastScout += t;

  // Check for engaged units, and send more units to assist
  map< AGameObject*, int > numAttackers;
  for( int i = 0; i < Game->gm->teams.size(); i++ )
  {
    Team *oTeam = Game->gm->teams[i];
    if( oTeam == this || oTeam->alliance == Alliance::Neutral ) continue; // skip same || neutral team
    
    for( int j = 0; j < oTeam->units.size(); j++ )
    {
      // Look for units engaged with members of this team
      // from opposing teams.
      AGameObject* g = oTeam->units[j];

      // If the attack target of the opponent piece is for a unit on this team, count it.
      if( g->attackTarget && g->attackTarget->team == this )
        numAttackers[ g->attackTarget ]++;
    }
  }

  // try and protect most victimized unit by fighting in its vicinity
  // Send units towards areas where they are needed.
  AGameObject *target = 0;
  int most = 0;
  for( pair< AGameObject*, int > p : numAttackers )
  {
    if( p.second > most )
    {
      target = p.first;
      most = p.second;
    }
  }

  // Send the units after most attacked unit on your team
  for( int i = 0; i < units.size(); i++ )
    if( !units[i]->attackTarget )
      units[i]->SetDestination( target->Pos() );

  // Scout militia if they aren't engaged
  if( ai.timeSinceLastScout > ai.scoutInterval )
  {
    // gather a group of unbusy militia to scout
    int groupSize = randInt( 2, 3 );
    vector<AGameObject*> group;
    for( int i = 0; i < units.size() && group.size() < groupSize; i++ )
      if( !units[i]->attackTarget )
        group.push_back( units[i] );
    // Find a random location on the map, and send the group off towards it
    FBox box = Game->flycam->floor->GetComponentsBoundingBox();
    FVector randomLocation = Rand( box.Min, box.Max );
    randomLocation.Z = box.Max.Z;

    ai.timeSinceLastScout = 0.f;
  }

  // The AI function sends off a group of units to scout if it is time for that
  // Check food ratios
  if( GetNumberOf( Types::BLDGTOWNHALL ) < 1   &&   CanAfford( Types::BLDGTOWNHALL ) )
  {
    Construct( Types::BLDGTOWNHALL );
  }

  // Do we need a farm?
  // First check to see if we need food. If we do need food,
  // and we have the resources to construct a Farm, then make one.
  // 1. Evaluate food requirements for team,
  else if( isNeedsFood()   &&   CanAfford( Types::BLDGFARM ) )
  {
    //UE_LOG( LogTemp, Warning, TEXT("Team %s is building a farm"), *team.Name );
    // request the team build a farm.
    // Try and assign the farm building to an available peasant
    Construct( Types::BLDGFARM );
  }
  
  else if( GetNumberOf( Types::BLDGBARRACKS )   &&   CanAfford( Types::BLDGBARRACKS ) )
  {
    Construct( Types::BLDGBARRACKS );
  }
}
