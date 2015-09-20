#pragma once

#include <vector>
#include <set>
using namespace std;

#include "AI.h"
#include "Types.h"
#include "UnitsData.h"

class AGameObject;
class AUnit;
class APeasant;
class ACombatUnit;

enum Alliances
{
  Neutral=0, Friendly=1, Enemy=2
};

// Team consists of a single player's 
struct Team
{
  int teamId;
  FString Name;
  // Alliance for this team wrt the local player.
  Alliances alliance;
  FAI ai;  // The AI controller for this team.
  FLinearColor Color;
  float Gold, Lumber, Stone;
  // How much damage an object should have before starting repair on it
  float DamageRepairThreshold;
  // Groups of buildings, peasants, and units sit in the same collection.
  vector<AGameObject*> units;
  int researchLevelMeleeWeapons, researchLevelArmor, researchLevelRangedWeapons;

  Team();
  Team( int iTeamId, FString str );
  Team( int iTeamId, FString str, Alliances iAlliance, FLinearColor color );
  void Defaults();
  bool isAllyTo( AGameObject* go );
  bool isEnemyTo( AGameObject* go );
  APeasant* GetNextAvailablePeasant();
  vector<ACombatUnit*> GetWarriors();

  void RemoveUnit( AGameObject *go );
  void OnMapLoaded();

  // Check if this team can afford to build a unit of UnitType
  bool Has( Types objectType );
  bool CanAfford( Types buildingType );
  bool CanBuild( Types buildingType );
  bool Spend( Types buildingType );

  // Usage of food by units in the game.
  int computeFoodUsage();
  int computeFoodSupply();
  int GetNumberOf( Types type );
  AGameObject* GetFirstOfType( Types type );
  FVector GetTownCentroid();

  // Tells if AI computes that this team 
  bool isNeedsFood();
  Types GetNeededResourceType();
  void runAI( float t );
  void Move( float t );
};

struct Alliance
{
  vector<Team*> teams;
  Alliances alliance;

  Alliance()
  {
    alliance = Alliances::Neutral;
  }
};