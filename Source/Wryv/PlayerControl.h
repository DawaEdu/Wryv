#pragma once

#include "UnitsData.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"
#include <vector>
using namespace std;
#include "PlayerControl.generated.h"

UCLASS()
class WRYV_API APlayerControl : public APlayerController
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

  bool IsKeyDown( FKey key );
  bool IsAnyKeyDown( vector<FKey> key );
  virtual void Tick( float t ) override;

};
