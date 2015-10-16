#pragma once

#include <map>
#include <set>
#include <vector>
using namespace std;

#include "GameFramework/GameMode.h"

#include "AIProfile.h"
#include "Enums.h"
#include "Team.h"
#include "WryvGameMode.generated.h"

// We'll use the GameMode object to store all information about
// the currently loaded game instance.
class UAIProfile;
class ACombatUnit;

UCLASS()
class WRYV_API AWryvGameMode : public AGameMode
{
  GENERATED_BODY()
public:
  // Launch state
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = GameMode ) TEnumAsByte< GameStates > state;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = GameMode ) TArray< FLinearColor > TeamColors;

  // VIRTUAL SECOND: RTS uses a virtual second to get deterministic play.
  float T; // The Timestep size of each frame.
  int GameSpeed; // Number of Ticks per frame. (increase sim speed).
  float Time; // elapsed gametime (seconds)
  // Gets you the timestep size given current game speed.
  uint64 tick; // The tick number.
  // Groups of teams.. this is the stock location for the Team objects.
  //vector< Alliance > alliances;
  vector< Team* > teams;
  Team *neutralTeam, *playersTeam, *enemyTeam;
  
  AWryvGameMode(const FObjectInitializer& PCIP);
  virtual void InitGame( const FString& MapName, const FString& Options, FString& ErrorMessage ) override;
  virtual void StartPlay();
  virtual void StartMatch();
  virtual void SetMatchState(FName NewState);
  virtual void Tick(float DeltaSeconds) override;
  vector<AGameObject*> GetObjectsOfType( UClass* ClassType );
  Alliance GetAlliance( Alliances alliance );
  // Finds all objects of type listed
  template<typename T> vector<T*> Find()
  {
    vector<T*> results;
    ULevel* level = GetWorld()->GetLevel(0);
    TTransArray<AActor*> *actors = &level->Actors;
    for( int i = 0; i < actors->Num() && !floor; i++ )
    {
      AActor* a = (*actors)[i];
      if( a == nullptr )  continue;
      if( T* t = Cast< T >( a ) )  results.push_back( t );
    }
    return results;
  }
  // Find by name.
  AActor* Find( FString name );
  virtual void BeginDestroy();
};
