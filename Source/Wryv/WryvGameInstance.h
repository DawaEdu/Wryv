#pragma once

#include <map>
using namespace std;

#include "Engine/GameInstance.h"

#include "AI.h"
#include "Command.h"
#include "GlobalFunctions.h"
#include "Team.h"
#include "UnitsData.h"
#include "WryvGameInstance.generated.h"

class AFlyCam;
class APlayerControl;
class AShape;
class ATheHUD;
class AWryvGameMode;
class AWryvGameState;
struct Team;

UCLASS()
class WRYV_API UWryvGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UnitData )  UDataTable* DataTable;
  // These are the flags by CommandID=>Flag instance, for flag removal
  map< int64, AShape* > Flags;
  int64 NextObjectID;
  // Cached copies of stock BaseUnitsData (FUnitsDataRow objects) for each UClass* type, loaded on-demand.
  // Used for things like the CanAfford() function, so we can look-ahead the
  // cost of spawning an object before actually spawning it.
private:
  map<UClass*,FUnitsData> BaseUnitsData;
public:
  ATheHUD *hud;
  APlayerControl *pc;
  AWryvGameMode *gm;
  AWryvGameState *gs;
  AFlyCam *flycam;
  bool IsDestroyStarted;
  deque<Command> commands;
  FActorSpawnParameters defaultSpawnParams;
  
  UWryvGameInstance(const FObjectInitializer& PCIP);
  // Sets the command for a specific object, clearing previous queue
  void SetCommand( const Command& cmd );
  // Queues a command for an object
  void EnqueueCommand( const Command& cmd );
  // Clear waypointed flags
  void ClearFlags();
  void ClearFlag( int64 cmdId );
  int64 NextId();
  AGameObject* GetUnitById( int64 unitId );
  bool IsReady();
  virtual void Init() override;
  virtual ULocalPlayer*	CreateInitialPlayer(FString& OutError) override;
  virtual void StartGameInstance() override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Out = *GLog) override;
  // T Must be a AGameObject class derivative.
  template <typename T> T* Make( TSubclassOf<AGameObject> ClassType, Team* team, FVector v )
  {
    if( !team )
    {
      //info( FS( "Make(): team was null when making unit of type %s!", *ClassType->GetName() ) );
      // select the neutral team.
      team = gm->neutralTeam;
    }
    if( !ClassType )
    {
      error( FS( "Make(): ClassType was NULL" ) );
      return 0;
    }
    T* obj = GetWorld()->SpawnActor<T>(
      ClassType, v, FRotator(0.f), defaultSpawnParams );
    if( obj )
    {
      obj->SetTeam( team );
    }
    else
    {
      error( FS( 
        "Object of type %s could not be spawned (did it die on spawn by colliding with another actor?)",
        *ClassType->GetName() ) );
      UE_LOG( LogTemp, Fatal, TEXT( "OBJECT OF CLASSTYPE %s "
        "COULD NOT BE SPAWNED: DID IT COLLIDE? CHECK POS!=0" ), *ClassType->GetName() );
    }
    return obj;
  }
  template <typename T> T* Make( TSubclassOf<AGameObject> ClassType, Team* team ) {
    return Make<T>( ClassType, team, FVector( 0.f ) );
  }
  template <typename T> T* Make( TSubclassOf<AGameObject> ClassType ) {
    return Make<T>( ClassType, 0, FVector( 0.f ) );
  }
public:
  FUnitsData GetData( UClass* ClassType );
  UTexture* GetPortrait( UClass* ClassType );
  virtual void BeginDestroy() override;
};

// This is the superglobal game instance
extern UWryvGameInstance* Game;


