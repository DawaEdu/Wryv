#pragma once

#include <vector>
#include <set>
#include <map>
using namespace std;

#include "GameFramework/HUD.h"
//#include "Runtime/MediaAssets/Public/MediaTexture.h"
#include "Runtime/Media/Public/IMediaPlayer.h"

#include "CursorTexture.h"
#include "Team.h"
#include "UnitAction.h"
#include "Widget.h"
#include "TheHUD.generated.h"

class FAssetRegistryModule;
class CostWidget;
class UCastSpellAction;
class AFlyCam;
class AGameObject;
class ImageWidget;
class ITextWidget;
class UMediaTexture;
class SlotPalette;
class StackPanel;
class APlayerControl;
class UserInterface;
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
  map<UClass*, UTexture*> ResourceIcons; // Maps UClasses to UTexture*

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* TitleScreenTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* TitleLogoTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* RightPanelTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* SlotPaletteTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* VoronoiBackground;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* PauseButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* ResumeButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* BuildButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* SolidWhiteTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* NoTextureTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterialInstance* ClockMaterialInstance;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterial* ClockMaterial;
  
  UPROPERTY() TArray<UMaterialInstanceDynamic*> MaterialInstances; // Referenced collection of material instances.
  // Required to prevent auto-cleanup of instanced materials
  // The blueprint for the fog of war instance to use.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* MapSlotEntryBackgroundTexture;
  
  // Render-to-texture target. Created inside the editor.
  USceneCaptureComponent2D *rendererIcon, *rendererMinimap;

  // Because canvas has to be valid for box selection to work it seems
  vector<AGameObject*> Selected;
  
  // The buttons currently showing on the user interface.
  UserInterface* ui;  // The root UI widget. It doesn't have a viz, but it parents all other display containers.
  bool Init;          // Global init for all objects
  FBox2D selectBox;
  bool SkipNextMouseUp; // for UI commands that absorb the mouse up.

  // The uClass of the selected object highlight
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D* PortraitTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D* MinimapTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* MediaTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterialInterface* MediaMaterial;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterial* WarBlot;
  //UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMediaPlayer* mediaPlayer;

  // The font used to render the text in the HUD.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *smallFont;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *mediumFont;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *largeFont;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FLinearColor EmptyCrosshairColor;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FLinearColor HitCrosshairColor;

  UCastSpellAction* NextSpell;
  Abilities NextAbility; //Queued ability

  ATheHUD(const FObjectInitializer& PCIP);
  virtual void PostInitializeComponents() override;
  virtual void BeginPlay() override;
  void InitWidgets();
  enum CursorType { CrossHairs, Hand };
  void SetCursorStyle( CursorType style, FLinearColor color );
  void SetHitCursor();
  void SetPointer();
  void SetNextAbility( Abilities nextAbility );
  TArray<FAssetData> ScanFolder( FName folder );
  
  HotSpot* MouseMoved( FVector2D mouse );
  HotSpot* MouseUpLeft( FVector2D mouse );
  HotSpot* MouseDownLeft( FVector2D mouse );
  void Select( vector<AGameObject*> objects );
  void Unselect( vector<AGameObject*> objects );
  void Status( FString msg );
  void UpdateMouse();

  bool isValid() { return IsCanvasValid_WarnIfNot() ; }

  void DrawMaterial(UCanvas* Canvas, UMaterialInterface* Material, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float MaterialU, float MaterialV, float MaterialUWidth, float MaterialVHeight);
  void DrawMaterial(UCanvas* Canvas, UMaterialInterface* Material, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float MaterialU, float MaterialV, float MaterialUWidth, float MaterialVHeight, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot);
  void DrawTexture(UCanvas* Canvas, UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor TintColor=FLinearColor::White, EBlendMode BlendMode=BLEND_Translucent, float Scale=1.f, bool bScalePosition=false, float Rotation=0.f, FVector2D RotPivot=FVector2D::ZeroVector);
  
  void RenderPortrait();
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
  void RenderScreen( USceneCaptureComponent2D* renderer, FVector lookPos, float radiusWorldUnits, FVector cameraDir );
  virtual void BeginDestroy() override;
};
