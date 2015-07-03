#pragma once

#include "UnitsData.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"
#include <vector>
using namespace std;
#include "PlayerControl.generated.h"

struct FWidgetData;

UCLASS()
class RTSGAME_API APlayerControl : public APlayerController
{
	GENERATED_UCLASS_BODY()
public:
  FSceneView* GetSceneView(ULocalPlayer* LocalPlayer);
  bool TraceMulti(const FVector2D ScreenPosition, vector<FHitResult>& HitResult);
	bool Trace(FVector2D ScreenPosition, AActor* actor, FHitResult& hit);
};
