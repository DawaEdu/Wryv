#include "Wryv.h"

#include "Barracks.h"
#include "Building.h"
#include "CombatUnit.h"
#include "Farm.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Peasant.h"
#include "Resource.h"
#include "Team.h"
#include "Townhall.h"
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
  Stone = 100;
  DamageRepairThreshold = 2.f/3.f;
  alliance = Neutral;
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
      if( p->Idling() )
        return p;
  return 0;
}

vector<APeasant*> Team::GetPeasants()
{
  vector<APeasant*> peasants;
  for( int i = 0; i < units.size(); i++ )
    if( APeasant* p = Cast<APeasant>( units[i] ) )
      peasants.push_back( p );
  return peasants;
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

bool Team::Has( UClass* ClassType )
{
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->IsA( ClassType ) )
     return 1;
  return 0;
}

bool Team::CanAfford( UClass* ClassType )
{
  FUnitsData data = Game->GetData( ClassType );
  return Gold >= data.GoldCost && Lumber >= data.LumberCost && Stone >= data.StoneCost;
}

bool Team::CanBuild( UClass* ClassType )
{
  // Requirements for building an object of a certain type.
  // Must have an object of required types to build, as well as afford.
  TArray< TSubclassOf< ABuilding > > requirements = Game->GetData( ClassType ).Requirements;
  for( int i = 0; i < requirements.Num(); i++ )
  {
    if( !Has( requirements[i] ) )
      return 0;
  }
  return 1; // Had all requirements.
}

bool Team::Spend( UClass* ClassType )
{
  if( !CanAfford( ClassType ) )
  {
    warning( FS( "Team %s cannot afford a %s", *Name, *ClassType->GetName() ) );
    return 0;
  }

  FUnitsData data = Game->GetData( ClassType );
  Gold   -= data.GoldCost;
  Lumber -= data.LumberCost;
  Stone  -= data.StoneCost;
  return 1;
}

bool Team::Refund( UClass* ClassType )
{
  // Refund cost of ClassType back to team.
  FUnitsData data = Game->GetData( ClassType );
  Gold   += data.GoldCost;
  Lumber += data.LumberCost;
  Stone  += data.StoneCost;
  return 1;
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
    if( ABuilding* building = Cast<ABuilding>( units[i] ) )
      foodSupply += building->FoodProvided;
  return foodSupply;
}

int Team::GetNumberOf( UClass* ClassType )
{
  int count = 0;
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->IsA( ClassType ) )
      count++;
  return count;
}

AGameObject* Team::GetFirstOfType( UClass* ClassType )
{
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->IsA( ClassType ) )
      return units[i];
  return 0; // not found
}

// Get the centroid of the town (avg position of all buildings)
FVector Team::GetTownCentroid()
{
  FVector v;
  for( int i = 0; i < units.size(); i++ )
    if( units[i]->IsA( ATownhall::StaticClass() ) )
      v += units[i]->Pos;
  return v;
}

bool Team::isNeedsFood()
{
  float ratio = (float)computeFoodUsage() / computeFoodSupply();
  return ratio > ai.foodFraction;
}

vector< TSubclassOf<AResource> > Team::GetNeededResourceTypes()
{
  return ai.GetNeededResourceTypes( *this );
}

AGameObject* Team::GetMostAttackedUnit()
{
  AGameObject* mostAttacked = 0;
  int maxAttackers = 0;
  for( int i = 0; i < units.size(); i++ )
  {
    if( units[i]->Attackers.size() > maxAttackers )
    {
      mostAttacked = units[i];
      maxAttackers = units[i]->Attackers.size();
    }
  }
  return mostAttacked;
}

void Team::runAI( float t )
{
  ai.timeSinceLastScout += t;

  // 1. Get all the idle peasants and assign them to resource gathering.
  vector<APeasant*> peasants = GetPeasants();
  for( APeasant* peasant : peasants )
  {
    if( peasant->Idling()   &&   !peasant->IsCarrying() )
    {
      vector< TSubclassOf<AResource> > resType = GetNeededResourceTypes();
      info( FS( "Peasant @ %f %f %f", peasant->Pos.X, peasant->Pos.Y, peasant->Pos.Z ) );
      AResource *res = peasant->FindAndTargetNewResource( peasant->Pos, resType, peasant->Stats.SightRange );
      if( res )
      {
        //info( FS( "Peasant %s assigned to gather %s", *peasant->Stats.Name, *res->Stats.Name ) );
      }
      else
      {
        info( FS( "Peasant %s couldn't find resources of any type to gather", *peasant->GetName() ) );
      }
    }
  }

  // Deal with CombatUnits
  vector<ACombatUnit*> combatUnits = GetWarriors();

  // Check for engaged units, and send more units to assist
  AGameObject* mostAttacked = GetMostAttackedUnit();
  // Try and protect most victimized unit by fighting in its vicinity
  // Send units towards areas where they are needed.
  if( mostAttacked )
  {
    for( int i = 0; i < combatUnits.size(); i++ )
    {
      // If the unit is idling and within SightRange of target, then
      // target any unit near the mostAttacked unit (OR make a selection
      // based on the class in the Attackers array).
      if( combatUnits[i]->Idling() )
      {
        combatUnits[i]->AttackGroundPosition( mostAttacked->Pos );
      }
    }
  }

  // Scout militia if they aren't engaged
  if( ai.timeSinceLastScout > ai.scoutInterval )
  {
    // gather a group of unbusy militia to scout
    int groupSize = randInt( 2, 3 );
    vector<AGameObject*> scoutGroup;
    for( int i = 0; i < combatUnits.size() && scoutGroup.size() < groupSize; i++ )
      if( combatUnits[i]->Idling() )
        scoutGroup.push_back( combatUnits[i] );
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
  if( !Has( ATownhall::StaticClass() )   &&   CanAfford( ATownhall::StaticClass() ) )
  {
    //peasant->Build( ATownhall::StaticClass() );
  }

  // Do we need a farm?
  // First check to see if we need food. If we do need food,
  // and we have the resources to construct a Farm, then make one.
  // 1. Evaluate food requirements for team,
  else if( isNeedsFood()   &&   CanAfford( AFarm::StaticClass() ) )
  {
    //LOG( "Team %s is building a farm", *team.Name );
    // request the team build a farm.
    // Try and assign the farm building to an available peasant
    //peasant->Build( Types::BLDGFARM );
  }
  
  else if( !Has( ABarracks::StaticClass() )   &&   CanAfford( ABarracks::StaticClass() ) )
  {
    //peasant->Build( Types::BLDGBARRACKS );
  }

  
}

void Team::Move( float t )
{
  runAI( t );

  for( int i = 0; i < units.size(); i++ )
    units[i]->Move( t );
}

