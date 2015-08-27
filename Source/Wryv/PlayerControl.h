#pragma once

#include "UnitsData.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"
#include <vector>
#include <set>
using namespace std;
#include "Box2DU.h"
#include "PlayerControl.generated.h"

class AGameObject;

UCLASS()
class WRYV_API APlayerControl : public APlayerController
{
	GENERATED_UCLASS_BODY()
public:
  // Called to bind functionality to input
  virtual void SetupInputComponent() override;
	virtual void SetupInactiveStateInputComponent(UInputComponent* InComponent) override;
  
  ULocalPlayer* GetLocalPlayer();
	FSceneView* GetSceneView( FSceneViewFamilyContext& ViewFamily );
  bool Trace(const FVector2D& ScreenPosition, AActor* actor, FHitResult& hit);
  bool TraceMulti(const FVector2D& ScreenPosition, vector<FHitResult>& HitResult);
  set<AGameObject*> Pick( const FBox2DU& box );
  bool IsKeyDown( FKey key );
  bool IsAnyKeyDown( vector<FKey> key );
  virtual void Tick( float t ) override;

};
