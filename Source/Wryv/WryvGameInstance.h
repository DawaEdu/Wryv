#pragma once

#include <map>
using namespace std;

#include "Engine/GameInstance.h"
#include "UnitsData.h"
#include "Types.h"
#include "AI.h"
#include "Team.h"
#include "UnitTypeUClassPair.h"
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
  bool IsReady() { return !IsDestroyStarted && init && hud && pc && gm && gs && flycam; }
  virtual void Init() override;
  void LoadUClasses();
  virtual ULocalPlayer*	CreateInitialPlayer(FString& OutError) override;
  virtual void StartGameInstance() override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Out = *GLog) override;
  
  inline UClass* GetUClass( Types type ) {
    UClass* uclass = unitsData[type].uClass;
    check( uclass && "Make: uclass was null" );
    return uclass;
  }
  template <typename T> T* Make( Types type ) {
    return GetWorld()->SpawnActor<T>( GetUClass(type), FVector(0.f), FRotator(0.f) );
  }
  template <typename T> T* Make( Types type, FVector v ) {
    return GetWorld()->SpawnActor<T>( GetUClass(type), v, FRotator(0.f) );
  }
  template <typename T> T* Make( Types type, FVector v, int32 teamId ) {
    T* obj = GetWorld()->SpawnActor<T>( GetUClass(type), v, FRotator(0.f) );
    obj->SetTeam( teamId ); // must be AGameObject derivative
    return obj;
  }
  AGameObject* Make( Types type, FVector v, int teamId );
  FUnitsDataRow GetData( Types type ) { return unitsData[ type ]; }
  UTexture* GetPortrait( Types type ) { return unitsData[ type ].Portrait; }
  virtual void BeginDestroy() override;
};

// This is the superglobal game instance
extern UWryvGameInstance* Game;


