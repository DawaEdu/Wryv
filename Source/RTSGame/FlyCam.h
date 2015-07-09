#pragma once

#include <vector>
using namespace std;

#include "Types.h"
#include "GameFramework/Pawn.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "FlyCam.generated.h"

class AMyHUD;
class APlayerControl;
struct FWidgetData;
class AGameObject;
class AGroundPlane;
struct GraphNode;
struct Edge;
class Pathfinder;
class UDialogBox;
class UMenu;
class UTipsBox;

UCLASS()
class RTSGAME_API AFlyCam : public APawn
{
	GENERATED_BODY()
public:
  AGroundPlane *floor;
  AGameObject *ghost;
  Pathfinder *pathfinder;
  UCameraComponent *camera;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  int32 Rows;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  int32 Cols;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  bool VizGrid;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Pathfinding )  float CameraZ;
  
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  USoundBase* bkgMusic;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  USoundBase* buildingPlaced;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Sounds )  USoundAttenuation* soundAttenuationSettings;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* White;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* Red;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* Green;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Colors )  UMaterial* Yellow;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UIBoxes )  UClass* DialogBoxBlueprint;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UIBoxes )  UClass* MenuBlueprint;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UIBoxes )  UClass* TipsBoxBlueprint;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = UIBoxes )  TArray<FString> Tips;

  UAudioComponent* music;
  float sfxVolume; // volume at which new SFX are played
  UDialogBox *dialogBox;
  UMenu *menu;
  UTipsBox *tipsBox;
  int tipNumber;

  // For stats INFO on display
  bool setup;
  float movementSpeed;
  UFloatingPawnMovement *MovementComponent;
  vector<AActor*> viz;
  
  // Sets default values for this pawn's properties
	AFlyCam( const FObjectInitializer& PCIP );
  
  // Called to bind functionality to input
  virtual void SetupPlayerInputComponent( UInputComponent* InputComponent ) override;
  void InitializeDefaultPawnInputBindings();
  void MoveCameraZUp( float amount );
  void MoveCameraZDown( float amount );
  void SetCameraPosition( FVector2D perc );
  void NextTip();
  static void SetColor( AActor* a, UMaterial* mat );
  // Series of points to visualize
  void Visualize( FVector& v, UMaterial* color );
  void Visualize( vector<FVector>& v );
  AActor* MakeSphere( FVector center, float radius, UMaterial* color );
  AActor* MakeCube( FVector center, float radius, UMaterial* color );
  AActor* MakeLine( FVector a, FVector b, UMaterial* color );
  void DisplayMenu();
  FHitResult LOS( FVector p, FVector q, TArray<AActor*> ignoredActors );
  void InitLevel();
  FVector getRandomLocation();
  void Setup();

  FVector2D getMousePos();
  FHitResult getHitGeometry();
  vector<FHitResult> getAllHitGeometry();
  FVector getHitFloor();
  bool intersectsAny( AActor* actor );
  bool intersectsAny( AActor* actor, vector<AActor*>& except );
  bool intersectsAnyOfType( AActor* actor, vector<Types>& types );
  
  void FindFloor();
  void MouseClicked();
  void MouseRightClicked();
  void MouseMoved();
  void MouseMovedX( float amount );
  void MouseMovedY( float amount );
  
  void MoveForward( float amount );
  void MoveBack( float amount );
  void MoveRight( float amount );
  void MoveLeft( float amount );
  void debug( int slot, FColor color, FString mess );
  void setGhost( Types ut );
  
	virtual void BeginPlay() override;
	virtual void Tick( float t ) override;
	
};
