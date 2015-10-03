#pragma once

#include <vector>
#include <set>
using namespace std;

#include "GameFramework/PlayerController.h"
#include "Engine/EngineTypes.h"

#include "Box2DU.h"

#include "PlayerControl.generated.h"

class AGameObject;

UCLASS()
class WRYV_API APlayerControl : public APlayerController
{
	GENERATED_UCLASS_BODY()
public:
  // These are initialized during load to get you the channel on which
  // a profile resides.
  map< FString, ECollisionChannel > CollisionChannels;
  // Called to bind functionality to input
  virtual void SetupInputComponent() override;
	virtual void SetupInactiveStateInputComponent(UInputComponent* InComponent) override;
  
  // Traces against particular actor component from a screen pos
  FHitResult TraceAgainst( UPrimitiveComponent* component, const FVector2D& ScreenPosition );
  // Gets the ray-hit location on a particular actor (used for ground plane/landscape)
  FHitResult TraceAgainst( UPrimitiveComponent* component, const Ray& ray );
  FHitResult TraceAgainst( AActor* actor, const FVector2D& ScreenPosition );
  FHitResult TraceAgainst( AActor* actor, const Ray& ray );

  // 2Space, usually used for impact point
  FHitResult RayPickSingle( const FVector2D& ScreenPosition );
  FHitResult RayPickSingle( const Ray& ray );
  // Gets all objects that are traced thru by vector
  vector<AGameObject*> RayPickMulti( const FVector2D& ScreenPosition );
  vector<AGameObject*> RayPickMulti( const Ray& ray );
  vector<AGameObject*> FrustumPick( const FBox2DU& box );
  vector<AGameObject*> FrustumPick( const FBox2DU& box, 
    set< TSubclassOf<AGameObject> > AcceptedTypes, set< TSubclassOf<AGameObject> > NotTypes );

  // 3Space
  vector<AGameObject*> ShapePick( FVector pos, FCollisionShape shape );
  // The results are ordered by RADIAL DISTANCE to the src cylinder
  vector<AGameObject*> ShapePickExcept( FVector pos, FCollisionShape shape,
    set< TSubclassOf<AGameObject> > AcceptedTypes, set< TSubclassOf<AGameObject> > NotTypes );

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
