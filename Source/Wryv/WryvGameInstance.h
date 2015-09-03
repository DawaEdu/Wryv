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
#include "WryvGameInstance.generated.h"

class ATheHUD;
class APlayerControl;
class AWryvGameMode;
class AWryvGameState;
class AFlyCam;
struct Team;

UCLASS()
class WRYV_API UWryvGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UnitData )  UDataTable* DataTable;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UnitData )  TArray< FUnitTypeUClassPair > UnitTypeUClasses;

  map<Types,FUnitsDataRow> unitsData;
  bool init;
  ATheHUD *hud;
  APlayerControl *pc;
  AWryvGameMode *gm;
  AWryvGameState *gs;
  AFlyCam *flycam;
  bool IsDestroyStarted;
  
  UWryvGameInstance(const FObjectInitializer& PCIP);
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
  template <typename T> T* Make( Types type, FVector v, Team* team ) {
    if( !team )
    {
      LOG( "Make(): team was null!" );
      return 0;
    }

    if( type < 0 || type >= Types::MAX )
    {
      error( FS( "Type %d is OOB defined types", (int)type ) );
      type = NOTHING;
    }
    T* obj = GetWorld()->SpawnActor<T>( GetUClass(type), v, FRotator(0.f) );
    
    if( !obj ) error( FS( "Object of type %s could not be spawned (check cast?)", *GetTypesName( type ) ) );
    obj->SetTeam( team );
    
    return obj;
  }
  template <typename T> T* Make( Types type, Team* team ) {
    return Make<T>( type, FVector( 0.f ), team );
  }
  FUnitsDataRow GetData( Types type ) { return unitsData[ type ]; }
  UTexture* GetPortrait( Types type ) { return unitsData[ type ].Portrait; }
  virtual void BeginDestroy() override;
};

// This is the superglobal game instance
extern UWryvGameInstance* Game;


