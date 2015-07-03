#pragma once

#include <map>
using namespace std;

#include "Engine/GameInstance.h"
#include "UnitsData.h"
#include "Types.h"
#include "AI.h"
#include "Team.h"
#include "RTSGameInstance.generated.h"

class AMyHUD;
class APlayerControl;
class ARTSGameGameMode;
class ARTSGameState;
class AFlyCam;
struct Team;

UCLASS()
class RTSGAME_API URTSGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stats)
  UDataTable* DataTable;

  // Map Unit Type name => properties for that unit.
  bool init;

  // Raw C++ Data of a units stats & data.
  map<Types,FUnitsDataRow> unitsData;

  AMyHUD *myhud;
  APlayerControl *pc;
  ARTSGameGameMode *gm;
  ARTSGameState *gs;
  AFlyCam *flycam;
  int64 tick;
  bool IsDestroyStarted;

  URTSGameInstance(const FObjectInitializer& PCIP);
  virtual void BeginDestroy() override;
  bool IsReady();
  AGameObject* Make( Types type, FVector v, int teamId );
  ARTSGameGameMode* GetGameMode();
  virtual void Init() override;
  virtual ULocalPlayer*	CreateInitialPlayer(FString& OutError) override;
  virtual void StartGameInstance() override;
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Out = *GLog) override;
};

// This is the superglobal game instance
extern URTSGameInstance* Game;


