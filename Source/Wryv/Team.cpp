#include "Wryv.h"

#include "CombatUnit.h"
#include "GameObject.h"
#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "Peasant.h"
#include "Team.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

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

Team::Team( int iTeamId, FString str, Alliances iAlliance, FLinearColor color )
{
  Defaults();
  teamId = iTeamId;
  Name = str;
  alliance = iAlliance;
  Color = color;
}

void Team::Defaults()
{
  Gold = 500;
  Lumber = 500;
  Stone = 250;
  DamageRepairThreshold = 2.f/3.f;
  alliance = Neutral;
  researchLevelMeleeWeapons = researchLevelRangedWeapons = researchLevelArmor = 0;
  Color = FLinearColor::Blue;
}

bool Team::isAllyTo( AGameObject* go )
{
  return go->team == this;
}

bool Team::isEnemyTo( AGameObject* go )
{
  // Cannot be enemies with the neutral team,
  return alliance != Neutral   &&   go->team->alliance != Neutral &&
         this != go->team; // Teams not the same, so enemies.
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
  return ratio > ai.foodFraction;
}

Types Team::GetNeededResourceType()
{
  return Types::RESGOLD;
  //return ai.GetNeededResourceType( *this );
}

void Team::runAI( float t )
{
  ai.timeSinceLastScout += t;

  // Check for engaged units, and send more units to assist
  int numAttackers = 0;
  AGameObject* mostAttacked = 0;
  for( int i = 0; i < units.size(); i++ )
  {
    if( units[i]->Attackers.size() > numAttackers )
    {
      mostAttacked = units[i];
      numAttackers = units[i]->Attackers.size();
    }
  }

  // try and protect most victimized unit by fighting in its vicinity
  // Send units towards areas where they are needed.
  for( int i = 0; i < units.size(); i++ )
    if( !units[i]->AttackTarget )
      units[i]->SetDestination( mostAttacked->Pos );

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
    FBox box = Game->flycam->floor->GetBox();
    FVector randomLocation = Rand( box.Min, box.Max );
    randomLocation.Z = box.Max.Z;

    ai.timeSinceLastScout = 0.f;
  }

  // The AI function sends off a group of units to scout if it is time for that
  // Check food ratios

  // Construct needed buildings
  APeasant* peasant = GetNextAvailablePeasant();
  if( GetNumberOf( Types::BLDGTOWNHALL ) < 1   &&   CanAfford( Types::BLDGTOWNHALL ) )
  {
    //peasant->Build( Types::BLDGTOWNHALL );
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
    //peasant->Build( Types::BLDGFARM );
  }
  
  else if( GetNumberOf( Types::BLDGBARRACKS )   &&   CanAfford( Types::BLDGBARRACKS ) )
  {
    //peasant->Build( Types::BLDGBARRACKS );
  }
}

void Team::Move( float t )
{
  //runAI( t );
  for( int i = 0; i < units.size(); i++ )
    units[i]->Move( t );
}
