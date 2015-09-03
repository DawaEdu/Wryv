#pragma once

#include <vector>
#include <set>
#include <map>
using namespace std;

#include "Widget.h"
#include "UnitsData.h"
#include "Team.h"
#include "CursorTexture.h"
#include "GameFramework/HUD.h"
//#include "Runtime/MediaAssets/Public/MediaTexture.h"
#include "Runtime/Media/Public/IMediaPlayer.h"

#include "TheHUD.generated.h"

class APlayerControl;
class AFlyCam;
class AGameObject;
class SlotPalette;
class ITextWidget;
class CostWidget;
class ImageWidget;
class StackPanel;
class UserInterface;
class FAssetRegistryModule;
class UMediaTexture;
class AWidget3D;

UCLASS()
class WRYV_API ATheHUD : public AHUD
{
  GENERATED_BODY()
public:
  // These are the widgets for the UI icons:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FCursorTexture MouseCursorHand;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FCursorTexture MouseCursorCrossHairs;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* GoldIconTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* LumberIconTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* StoneIconTexture;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* TitleScreenTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* TitleLogoTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* RightPanelTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* SlotPaletteTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* StackPanelTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* TooltipBackgroundTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* PauseButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* ResumeButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* BuildButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* SolidWhiteTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* NoTextureTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterialInstance* ClockMaterialInstance;
  UPROPERTY() TArray<UMaterialInstanceDynamic*> MaterialInstances; // Referenced collection of material instances.
  // Required to prevent auto-cleanup of instanced materials
  // The blueprint for the fog of war instance to use.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* MapSlotEntryBackgroundTexture;
  
  // Render-to-texture target. Created inside the editor.
  USceneCaptureComponent2D *rendererIcon, *rendererMinimap;

  // Because canvas has to be valid for box selection to work it seems
  set<AGameObject*> Selected;
  FName AttackTargetName, FollowTargetName, SelectedTargetName;

  // The buttons currently showing on the user interface.
  UserInterface* ui;  // The root UI widget. It doesn't have a viz, but it parents all other display containers.
  bool Init;          // Global init for all objects
  FBox2D selectBox;

  // The uClass of the selected object highlight
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D* PortraitTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D* MinimapTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* MediaTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterialInterface* MediaMaterial;
  //UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMediaPlayer* mediaPlayer;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD )  UMaterial* WarBlot;

  // The font used to render the text in the HUD.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *smallFont;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *largeFont;

  Types NextAction;     // The next action to be taken by the UI, build building, move etc.
  
  // This is the currently displayed amount of gold,lumber,stone
  // These are state variables since they are refreshed each frame.
  float displayedGold, displayedLumber, displayedStone;
  
  ATheHUD(const FObjectInitializer& PCIP);
  virtual void BeginPlay() override;
  TArray<FAssetData> ScanFolder( FName folder );
  
  HotSpot* MouseMoved( FVector2D mouse );
  HotSpot* MouseUpLeft( FVector2D mouse );
  HotSpot* MouseDownLeft( FVector2D mouse );
  set<AGameObject*> Filter( set<AGameObject*> objects, set<Types> typesToRemove );
  void Select( set<AGameObject*> objects );
  void MarkAsSelected( AGameObject* object );
  void MarkAsFollow( AGameObject* object );
  void MarkAsAttack( AGameObject* object );
  
  void InitWidgets();
  void Setup();
  
  void UpdateDisplayedResources();
  void UpdateMouse();

  bool Valid() { return IsCanvasValid_WarnIfNot() ; }

  void DrawMaterial(UCanvas* Canvas, UMaterialInterface* Material, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float MaterialU, float MaterialV, float MaterialUWidth, float MaterialVHeight);
  void DrawMaterial(UCanvas* Canvas, UMaterialInterface* Material, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float MaterialU, float MaterialV, float MaterialUWidth, float MaterialVHeight, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot);
  void DrawTexture(UCanvas* Canvas, UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor TintColor=FLinearColor::White, EBlendMode BlendMode=BLEND_Translucent, float Scale=1.f, bool bScalePosition=false, float Rotation=0.f, FVector2D RotPivot=FVector2D::ZeroVector);
  
  void DrawPortrait();
  virtual void DrawHUD() override;

  virtual void Tick( float t );
  // Get the Z distance that an object should be placed
  // when it has a world radius of radiusWorldUnits
  // such that it has a rendered width of radiusOnScreenPx
  // on a texture of width texW when the fov is fovyDegrees
  // texW is required to compute clipSpace coordinates,
  // which is the coordinate system that the formula works in.
  float GetZDistance( float radiusWorldUnits, float radiusOnScreenPx, float texW, float fovyDegrees );

  // Compute the resultant pixel width of an object:
  //   - with radius radiusWorldUnits
  //   - at a distance distanceToObject units away from the camera
  // where the texture has width=texW and the camera
  // has FOV=fovyDegrees
  float GetPxWidth( float radiusWorldUnits, float distanceToObject, float texW, float fovyDegrees );
  void RenderScreen( USceneCaptureComponent2D* renderer, UTextureRenderTarget2D* tt, FVector objectPos, float radiusWorldUnits, FVector cameraDir );
  virtual void BeginDestroy() override;
};
