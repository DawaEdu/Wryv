#pragma once

#include <vector>
using namespace std;

#include "Types.h"
#include "GameFramework/Pawn.h"
#include "FogOfWar.h"
#include "FlyCam.generated.h"

class ATheHUD;
class APlayerControl;
class AGameObject;
struct GraphNode;
struct Edge;
class Pathfinder;

UCLASS()
class WRYV_API AFlyCam : public APawn
{
	GENERATED_BODY()
public:
  AActor *floor; // Every level must have a Landscape object called the floor.
  FBox floorBox; // we only find the floor's box once (at level start).
  AGameObject *ghost; // ghost of the building being set for placement
  Pathfinder *pathfinder;
  UCameraComponent* MainCamera; // UPROPERTY type listing doesn't make it appear in listing
  AFogOfWar* fogOfWar;  // The fog of war instance plane, constructed from the uclass listed above
  
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  int32 Rows;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  int32 Cols;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  bool VizGrid;
  
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  USoundBase* bkgMusic;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  USoundBase* buildingPlaced;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  USoundAttenuation* soundAttenuationSettings;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* White;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* Red;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* Green;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* Yellow;

  // the name of the title level
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UOptions )  FName LevelMap;

  UAudioComponent* music;
  float sfxVolume; // volume at which new SFX are played
  
  // this flag indicates if the level has been initialized yet
  bool setupLevel;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UOptions )  float CameraMovementSpeed;
  UFloatingPawnMovement *MovementComponent;
  UInputComponent* InputComponent;
  vector<AActor*> viz;
  
  // Sets default values for this pawn's properties
	AFlyCam( const FObjectInitializer& PCIP );
  
  virtual void BeginPlay() override;
	  
  // Called to bind functionality to input.
  // SetupPlayerInputComponent only exists in APawn, so we attach
  // functionality to each input component
  virtual void SetupPlayerInputComponent( UInputComponent* InputComponent ) override;
  
  // Start to load the map in levelName
  void LoadLevel( FName levelName );
  void OnLevelLoaded();
  void InitializePathfinding();
  void UnloadLevel();
  
  void SetCameraPosition( FVector2D perc );
  FHitResult LOS( FVector p, FVector q, TArray<AActor*> ignoredActors );
  static void SetColor( AActor* a, UMaterial* mat );
  // Series of points to visualize
  void Visualize( FVector& v, UMaterial* color );
  void Visualize( vector<FVector>& v );
  AActor* MakeSphere( FVector center, float radius, UMaterial* color );
  AActor* MakeCube( FVector center, float radius, UMaterial* color );
  AActor* MakeLine( FVector a, FVector b, UMaterial* color );
  void RetrievePointers();
  void debug( int slot, FColor color, FString mess );
  void setGhost( Types ut );
  
  FVector2D getMousePos();
  FHitResult getHitGeometry();
  vector<FHitResult> getAllHitGeometry();
  FVector getHitFloor(FVector eye, FVector look);
  FVector getHitFloor();
  bool intersectsAny( AActor* actor );
  bool intersectsAny( AActor* actor, vector<AActor*>& except );
  bool intersectsAnyOfType( AActor* actor, vector<Types>& types );
  
  void FindFloor();
  void MouseDownLeft();
  void MouseUpLeft();
  void MouseDownRight();
  void MouseUpRight();
  void MouseMoved();
  void MouseMovedX( float amount );
  void MouseMovedY( float amount );
  
  void MoveCameraZUp( float amount );
  void MoveCameraZDown( float amount );
  void MoveForward( float amount );
  void MoveBack( float amount );
  void MoveLeft( float amount );
  void MoveRight( float amount );
  
	virtual void Tick( float t ) override;
	
};
