#pragma once

#include <vector>
#include <set>
using namespace std;

#include "AI.h"

class ACombatUnit;
class AGameObject;
class APeasant;
class AResource;
class UResearch;
class AUnit;

enum Alliances
{
  Neutral=0, Friendly=1, Enemy=2
};

// The techtree lists buildings the AI WANTS to build
// as the game progresses. when the team has enough
// resources and units its starts building buildings
// down the techtree. when the building is built it is marked built,
// if it is destroyed it needs to be rebuilt with urgency as far
// up the techtree.
//   1. townhall

struct TechTree
{
  vector< TSubclassOf<AGameObject> > list;
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
  // The research level objects that have been completed. New units have
  // these stats pushed into their stats stack.
  vector<UResearch*> CompletedResearches;

  Team();
  Team( int iTeamId, FString str );
  Team( int iTeamId, FString str, Alliances iAlliance, FLinearColor color );
  void Defaults();
  bool isAllyTo( AGameObject* go );
  bool isEnemyTo( AGameObject* go );
  APeasant* GetNextAvailablePeasant();
  vector<APeasant*> GetPeasants();
  vector<ACombatUnit*> GetWarriors();

  void RemoveUnit( AGameObject *go );
  void OnMapLoaded();

  // Check if this team can afford to build a unit of ClassType
  bool Has( UClass* ClassType );
  bool CanAfford( UClass* ClassType );
  bool CanBuild( UClass* ClassType );
  bool Spend( UClass* ClassType );
  bool Refund( UClass* ClassType );
  void ResourceChange( int gold, int lumber, int stone );
  // Usage of food by units in the game.
  int computeFoodUsage();
  int computeFoodSupply();
  int GetNumberOf( UClass* ClassType );
  AGameObject* GetFirstOfType( UClass* ClassType );
  FVector GetTownCentroid();

  // Tells if AI computes that this team 
  bool isNeedsFood();
  vector< TSubclassOf<AResource> > GetNeededResourceTypes();
  AGameObject* GetMostAttackedUnit();
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


