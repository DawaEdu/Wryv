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

  // Traces against particular actor from a screen pos
  FHitResult TraceAgainst( AActor* actor, const FVector2D& ScreenPosition );
  // Gets the ray-hit location on a particular actor (used for ground plane/landscape)
  FHitResult TraceAgainst( AActor* actor, const FVector& eye, const FVector& lookDir );

  // Searches for closest actor based on screen pos
  FHitResult PickClosest( const FVector2D& ScreenPosition );
  FHitResult PickClosest( const FVector& eye, const FVector& lookDir );

  set<AGameObject*> Pick( FVector pos, FCollisionShape shape );

  // Checks if the gameobject collides with any type listed. Used for pathfinder construction.
  set<AGameObject*> Pick( AGameObject* object, UPrimitiveComponent* up );
  set<AGameObject*> PickExcept( AGameObject* object, UPrimitiveComponent* up, set<AGameObject*> except );
  
  // Gets all objects that are traced thru by vector
  set<AGameObject*> Pick( const FVector2D& ScreenPosition );
  set<AGameObject*> Pick( const FVector& eye, const FVector& lookDir );
  set<AGameObject*> Pick( const FBox2DU& box, set<Types> AcceptedTypes, set<Types> NotTypes );
  bool IsKeyDown( FKey key );
  bool IsAnyKeyDown( vector<FKey> key );
  virtual void Tick( float t ) override;

};
