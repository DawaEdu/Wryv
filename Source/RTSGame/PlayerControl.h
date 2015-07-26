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
  // Called to bind functionality to input
  virtual void SetupInputComponent() override;
	virtual void SetupInactiveStateInputComponent(UInputComponent* InComponent) override;
  void K();
  
  FSceneView* GetSceneView(ULocalPlayer* LocalPlayer);
  bool TraceMulti(const FVector2D ScreenPosition, vector<FHitResult>& HitResult);
	bool Trace(FVector2D ScreenPosition, AActor* actor, FHitResult& hit);

  // Gets you the view frustum's far corners
  vector<FVector> GetFrustumCorners();
  vector<FVector> GetFrustumIntersectionWith( AActor* actor );

  bool IsDown( FKey key );
  virtual void Tick( float t ) override;

};
