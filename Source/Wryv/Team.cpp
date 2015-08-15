#include "Wryv.h"
#include "Team.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
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
  researchLevelMeleeWeapons = researchLevelRangedWeapons = researchLevelArmor = 0;
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
    LOG( "Cannot construct %s, no available peasants!", *GetTypesName(buildingType) );
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
      p->aiPlaceBuildingAtRandomLocation( buildingType );
    }
  }
}

void Team::AddUnit( AGameObject *go )
{
  units.push_back( go );
  go->team = this;
}

void Team::RemoveUnit( AGameObject *go )
{
  removeElement( units, go );
  go->team = 0;
}

void Team::OnMapLoaded()
{
  // Call OnMapLoaded() for all gameobjects
  for( int i = 0; i < units.size(); i++ )
    units[i]->OnMapLoaded();
}

bool Team::Has( Types objectType )
{
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->Stats.Type == objectType )
     return 1;
  return 0;
}

bool Team::CanAfford( Types type )
{
  FUnitsDataRow ud = Game->unitsData[ type ];
  return Gold >= ud.GoldCost && Lumber >= ud.LumberCost && Stone >= ud.StoneCost;
}

bool Team::CanBuild( Types buildingType )
{
  // Requirements for building an object of a certain type.
  // Must have an object of required types to build, as well as afford.
  for( int i = 0; i < Game->unitsData[ buildingType ].Requirements.Num(); i++ )
    if( !Has( buildingType ) )
      return 0;
  return 1; // Had all required buildings.
}

bool Team::Spend( Types type )
{
  FUnitsDataRow ud = Game->unitsData[ type ];
  if( CanAfford( type ) )
  {
    Gold   -= ud.GoldCost;
    Lumber -= ud.LumberCost;
    Stone  -= ud.StoneCost;
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
    foodUse += units[i]->Stats.FoodUsed;
  return foodUse;
}

// Count by #farms instantiated
int Team::computeFoodSupply()
{
  int foodSupply = 0;
  // The food supply is from Farm objects
  for( int i = 0; i < units.size(); i++ )
    foodSupply += units[i]->Stats.FoodProvided;
  return foodSupply;
}

int Team::GetNumberOf( Types type )
{
  int count = 0;
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->Stats.Type == type )
      count++;
  return count;
}

AGameObject* Team::GetFirstOfType( Types type )
{
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->Stats.Type == type )
      return units[i];
  return 0; // not found
}

// Get the centroid of the town (avg position of all buildings)
FVector Team::GetTownCentroid()
{
  FVector v;
  for( int i = 0; i < units.size(); i++ )
    if( IsBuilding( units[i]->Stats.Type ) )
      v += units[i]->Pos;
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
    Team* oTeam = Game->gm->teams[i];
    if( oTeam == this || oTeam->alliance == Alliance::Neutral )
      continue; // skip same || neutral team
    
    for( int j = 0; j < oTeam->units.size(); j++ )
    {
      // Look for units engaged with members of this team from opposing teams.
      AGameObject* g = oTeam->units[j];

      // If the attack target of the opponent piece is for a unit on this team, count it.
      if( g->AttackTarget && g->AttackTarget->team == this )
        numAttackers[ g->AttackTarget ]++;
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
    if( !units[i]->AttackTarget )
      units[i]->SetDestination( target->Pos );

  // Scout militia if they aren't engaged
  if( ai.timeSinceLastScout > ai.scoutInterval )
  {
    // gather a group of unbusy militia to scout
    int groupSize = randInt( 2, 3 );
    vector<AGameObject*> group;
    for( int i = 0; i < units.size() && group.size() < groupSize; i++ )
      if( !units[i]->AttackTarget )
        group.push_back( units[i] );
    // Find a random location on the map, and send the group off towards it
    FBox box = Game->flycam->floorBox;
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
    //LOG( "Team %s is building a farm", *team.Name );
    // request the team build a farm.
    // Try and assign the farm building to an available peasant
    Construct( Types::BLDGFARM );
  }
  
  else if( GetNumberOf( Types::BLDGBARRACKS )   &&   CanAfford( Types::BLDGBARRACKS ) )
  {
    Construct( Types::BLDGBARRACKS );
  }
}

void Team::Move( float t )
{
  //runAI( t );
  for( int i = 0; i < units.size(); i++ )
    units[i]->Move( t );
}
