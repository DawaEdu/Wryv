#pragma once

#include <vector>
#include <map>
using namespace std;

#include "Team.h"
#include "GameFramework/GameMode.h"
#include "RTSGameGameMode.generated.h"

// We'll use the GameMode object to store all information about
// the currently loaded game instance.

UCLASS()
class RTSGAME_API ARTSGameGameMode : public AGameMode
{
  GENERATED_BODY()
public:
  enum GameState{
    Title, Menu, Running
  };
  GameState state;

  // Groups of teams.. this is the stock location for the Team objects.
  map< int32, Team* > teams;
  Team *neutralTeam, *playersTeam, *enemyTeam;
  
  ARTSGameGameMode(const FObjectInitializer& PCIP);
  virtual void BeginDestroy();
  virtual void StartPlay();
  virtual void InitGame( const FString& MapName, const FString& Options, FString& ErrorMessage ) override;
  virtual void Tick(float t) override;
  vector<AGameObject*> GetObjectsOfType( Types type );
};
