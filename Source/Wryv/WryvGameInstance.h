#pragma once

#include <map>
using namespace std;

#include "Engine/GameInstance.h"
#include "UnitsData.h"
#include "Types.h"
#include "AI.h"
#include "Team.h"
#include "UnitTypeUClassPair.h"
#include "GlobalFunctions.h"
#include "Command.h"
#include "WryvGameInstance.generated.h"

class ATheHUD;
class APlayerControl;
class AWryvGameMode;
class AWryvGameState;
class AFlyCam;
class AShape;
struct Team;

UCLASS()
class WRYV_API UWryvGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UnitData )  UDataTable* DataTable;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UnitData )  TArray< FUnitTypeUClassPair > UnitTypeUClasses;

  // These are the flags by CommandID=>Flag instance, for flag removal
  map< int64, AShape* > Flags;

  int64 NextObjectID;
  map<Types,FUnitsDataRow> unitsData;
  bool UClassesLoaded;
  ATheHUD *hud;
  APlayerControl *pc;
  AWryvGameMode *gm;
  AWryvGameState *gs;
  AFlyCam *flycam;
  bool IsDestroyStarted;
  deque<Command> commands;
  
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
  void AssertIntegrity();
  void LoadUClasses();
  virtual ULocalPlayer*	CreateInitialPlayer(FString& OutError) override;
  virtual void StartGameInstance() override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Out = *GLog) override;
  
  // Uses the UI-populated unit-types-UClasses array.
  inline UClass* GetUClass( Types type ) {
    int t = (int)type;
    if( t < 0 || t >= UnitTypeUClasses.Num() ) {
      error( FS( "Unit type %d / %s doesn't exist in UnitTypesUClasses mapping",
        t, *GetTypesName( type ) ) );
      t = NOTHING; // Return the nothing object instead
    }
    return UnitTypeUClasses[ t ].uClass;
  }
  // T Must be a AGameObject class derivative.
  template <typename T> T* Make( Types type, Team* team, FVector v ) {
    if( !team )
    {
      LOG( "Make(): team was null!" );
      // select the neutral team.
      team = gm->neutralTeam;
    }
    if( type < 0 || type >= Types::MAX )
    {
      error( FS( "Type %d is OOB defined types", (int)type ) );
      type = NOTHING;
    }
    UClass *uClass = GetUClass( type );
    if( !uClass )
    {
      error( FS( "Couldn't find UCLASS belonging to type %d", (int)type ) );
      return 0;
    }

    T* obj = GetWorld()->SpawnActor<T>( uClass, v, FRotator(0.f) );
    if( obj )
    {
      obj->SetTeam( team );
    }
    else
    {
      error( FS( "Object of type %s could not be spawned (did it die on spawn by colliding with another actor?)", *GetTypesName( type ) ) );
    }
    
    return obj;
  }
  template <typename T> T* Make( Types type, Team* team ) {
    return Make<T>( type, team, FVector( 0.f ) );
  }
  FUnitsDataRow GetData( Types type ) { return unitsData[ type ]; }
  UTexture* GetPortrait( Types type ) { return unitsData[ type ].Portrait; }
  virtual void BeginDestroy() override;
};

// This is the superglobal game instance
extern UWryvGameInstance* Game;


