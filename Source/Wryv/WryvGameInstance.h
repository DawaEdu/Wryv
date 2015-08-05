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
struct FUnitTypeUClassPair;

UCLASS()
class WRYV_API UWryvGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UnitData )  UDataTable* DataTable;

  // This contains the stock mapping from Types:: enum to
  // the UClass used for that Type of object in-game.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UnitData ) 
  TArray< FUnitTypeUClassPair > UnitTypeUClasses;

  // Raw C++ Data of a units stats & data.
  map<Types,FUnitsDataRow> unitsData;
  bool init;
  ATheHUD *hud;
  APlayerControl *pc;
  AWryvGameMode *gm;
  AWryvGameState *gs;
  AFlyCam *flycam;
  bool IsDestroyStarted;
  
  UWryvGameInstance(const FObjectInitializer& PCIP);
  virtual void BeginDestroy() override;
  bool IsReady();
  AGameObject* Make( Types type, FVector v, int teamId );
  AWryvGameMode* GetGameMode();
  virtual void Init() override;
  void LoadUClasses();
  virtual ULocalPlayer*	CreateInitialPlayer(FString& OutError) override;
  virtual void StartGameInstance() override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Out = *GLog) override;
};

// This is the superglobal game instance
extern UWryvGameInstance* Game;


