#pragma once

#include <vector>
using namespace std;

#include "AI.h"
#include "Types.h"
#include "UnitsData.h"

class AGameObject;
class AUnit;
class APeasant;
class ACombatUnit;

enum Alliance
{
  Neutral, Friendly, Enemy
};

struct Team
{
  int teamId;
  FString Name;
  // Alliance for this team wrt the local player.
  Alliance alliance;
  AI ai;  // The AI controller for this team.
  FLinearColor color;
  float Gold, Lumber, Stone;
  // How much damage an object should have before starting repair on it
  float DamageRepairThreshold;
  // Groups of buildings, peasants, and units sit in the same collection.
  vector<AGameObject*> units;
  int researchLevelMeleeWeapons, researchLevelArmor, researchLevelRangedWeapons;

  Team();
  Team( int iTeamId, FString str );
  void Defaults();
  APeasant* GetNextAvailablePeasant();
  vector<ACombatUnit*> GetWarriors();
  void Construct( Types type );

  void AddUnit( AGameObject *go );
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

