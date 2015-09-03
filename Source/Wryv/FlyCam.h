#pragma once

#include <vector>
#include <set>
using namespace std;

#include <Types.h>
#include <GameFramework/Pawn.h>
#include "FogOfWar.h"
#include "UISounds.h"
#include "SoundEffect.h"
#include "FlyCam.generated.h"

class ATheHUD;
class APlayerControl;
class AGameObject;
struct GraphNode;
struct Edge;
class Pathfinder;
class AGroundPlane;

inline bool operator<( const FLinearColor& c1, const FLinearColor& c2 )
{
  return c1.Quantize().AlignmentDummy < c2.Quantize().AlignmentDummy;
}

UCLASS()
class WRYV_API AFlyCam : public APawn
{
	GENERATED_BODY()
public:
  AGroundPlane* floor; // Every level must have an object called the floor.
  FBox floorBox;       // we only find the floor's box once (at level start).
  AGameObject* ghost;  // ghost of the building being set for placement
  Pathfinder* pathfinder;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Cam )  USceneComponent* DummyRoot;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Cam )  UCameraComponent* MainCamera; // UPROPERTY type listing doesn't make it appear in listing
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Cam )  UCameraComponent* OrthoCam;
  AFogOfWar* fogOfWar;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  int32 Rows;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  int32 Cols;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  bool VizGrid;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  float FloorBoxTraceFraction;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  float CheckerSphereRadius;

  // Use a TARRAY to keep sfx organized, includes music & effects
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  TArray<FSoundEffect> SFX;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterialInterface* BaseWhiteInterface;
  //UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* BaseWhiteMaterial;
  UPROPERTY() TArray<UMaterialInstanceDynamic*> ColorMaterials; //Reference counted collection of created colors.
  map<FLinearColor, UMaterialInstanceDynamic*> Colors;

  // the name of the title level
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UOptions )  FName LevelMap;

  UAudioComponent* music;
  float sfxVolume; // volume at which new SFX are played
  
  // this flag indicates if the level has been initialized yet
  bool setupLevel;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UOptions )  float CameraMovementSpeed;
  UFloatingPawnMovement *MovementComponent;
  UInputComponent* InputComponent;
  vector<AGameObject*> viz;
  
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
  UMaterialInterface* GetMaterial( FLinearColor color );
  // Series of points to visualize
  void Visualize( Types type, FVector& v, float s, FLinearColor color );
  void Visualize( Types type, vector<FVector>& v, float s, FLinearColor startColor, FLinearColor endColor );
  void ClearViz();
  
  AGameObject* MakeLine( FVector a, FVector b, FLinearColor color );
  void RetrievePointers();
  void debug( int slot, FColor color, FString mess );
  void setGhost( Types ut );
  
  FVector2D getMousePos();
  // Modifies vector if can hit ground. If not, it just stays floating.
  bool SetOnGround( FVector& v );
  
  void FindFloor();
  void Select( set<AGameObject*> objects );
  void MouseUpLeft();
  void MouseDownLeft();
  void MouseUpRight();
  void MouseDownRight();
  void MouseMoved();
  void MouseMovedX( float amount );
  void MouseMovedY( float amount );
  
  void MoveCameraZUp( float amount );
  void MoveCameraZDown( float amount );
  void MoveForward( float amount );
  void MoveBack( float amount );
  void MoveLeft( float amount );
  void MoveRight( float amount );
  void PlaySound( UISounds sound ){
    UGameplayStatics::PlaySoundAttached( SFX[ sound ].Sound, RootComponent );
  }

	virtual void Tick( float t ) override;
	
};
