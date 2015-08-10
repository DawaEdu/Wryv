#pragma once

#include <vector>
#include <map>
using namespace std;

#include "Types.h"
#include "Team.h"
#include "GameFramework/GameMode.h"
#include "WryvGameMode.generated.h"

// We'll use the GameMode object to store all information about
// the currently loaded game instance.

UCLASS()
class WRYV_API AWryvGameMode : public AGameMode
{
  GENERATED_BODY()
public:
  // Launch state
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) TEnumAsByte< GameStates > state;

  // VIRTUAL SECOND: RTS uses a virtual second to get deterministic play.
  float T; // The Timestep size of each frame.
  int GameSpeed; // Number of Ticks per frame. (increase sim speed).
  float Time; // elapsed gametime (seconds)
  // Gets you the timestep size given current game speed.
  
  uint64 tick; // The tick number.
  
  // Groups of teams.. this is the stock location for the Team objects.
  map< int32, Team* > teams;
  Team *neutralTeam, *playersTeam, *enemyTeam;
  
  AWryvGameMode(const FObjectInitializer& PCIP);
  virtual void BeginDestroy();
  virtual void StartPlay();
  virtual void InitGame( const FString& MapName, const FString& Options, FString& ErrorMessage ) override;
  virtual void Tick(float DeltaSeconds) override;
  vector<AGameObject*> GetObjectsOfType( Types type );
};
