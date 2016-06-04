#pragma once

#include <vector>
#include <set>
using namespace std;

#include "Box2DU.h"
#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"

#include "PlayerControl.generated.h"

class AGameObject;

typedef set< TSubclassOf<AGameObject> > /* as simply */ SetAGameObject;

UCLASS()
class WRYV_API APlayerControl : public APlayerController
{
  GENERATED_BODY()
public:
  const static float RayLength;

  // These are initialized during load to get you the channel on which
  // a profile resides.
  map< FString, ECollisionChannel > CollisionChannels;
  
  APlayerControl( const FObjectInitializer& PCIP );
  // Called to bind functionality to input
  virtual void SetupInputComponent() override;
	virtual void SetupInactiveStateInputComponent(UInputComponent* InComponent) override;
  
  // Filters single Actor to exclude if NOT in acceptable types, and also
  // exclude if IS of unacceptable types.
  AGameObject* Filter( AActor* actor, SetAGameObject AcceptedTypes, SetAGameObject NotTypes );
  // Filters a group of actors to see if they meet selection criteria
  vector<AGameObject*> Filter( const TArray<FHitResult>& hits, SetAGameObject AcceptedTypes, SetAGameObject NotTypes );

  // Traces against particular actor component from a screen pos
  FHitResult TraceAgainst( UPrimitiveComponent* component, const FVector2D& ScreenPosition );
  // Gets the ray-hit location on a particular actor (used for ground plane/landscape)
  FHitResult TraceAgainst( UPrimitiveComponent* component, const Ray& ray );
  FHitResult TraceAgainst( AActor* actor, const FVector2D& ScreenPosition );
  FHitResult TraceAgainst( AActor* actor, const Ray& ray );

  // 2Space, usually used for impact point
  FHitResult RayPickSingle( const FVector2D& ScreenPosition, SetAGameObject AcceptedTypes, SetAGameObject NotTypes );
  FHitResult RayPickSingle( const Ray& ray, SetAGameObject AcceptedTypes, SetAGameObject NotTypes );
  // Gets all objects that are traced thru by vector
  vector<AGameObject*> RayPickMulti( const FVector2D& ScreenPosition, SetAGameObject AcceptedTypes, SetAGameObject NotTypes );
  vector<AGameObject*> RayPickMulti( const Ray& ray, SetAGameObject AcceptedTypes, SetAGameObject NotTypes );

  vector<Ray> GetFrustumRays( const FBox2DU& box );
  vector<AGameObject*> FrustumPick( const FBox2DU& box, SetAGameObject AcceptedTypes, SetAGameObject NotTypes );

  // 3Space
  // The results are ordered by RADIAL DISTANCE to the src cylinder
  vector<AGameObject*> ShapePick( FVector pos, FCollisionShape shape,
    SetAGameObject AcceptedTypes, SetAGameObject NotTypes );

  // Checks if the gameobject collides with any type listed. Used for pathfinder construction.
  // This uses the native Collision Profile of the PRIMITIVE used to query.
  // You can specify ignored actors using this version of the function
  vector<AGameObject*> ComponentPickExcept( AGameObject* object, UPrimitiveComponent* up,
    vector<AGameObject*> except,
    // Eg "Checkers" looking for any { "BuildingMesh", "ResourceMesh" }
    FString queryObjectType, vector<FString> intersectableTypes  );

  bool IsKeyDown( FKey key );
  bool IsAnyKeyDown( vector<FKey> key );
  virtual void Tick( float t ) override;

};
