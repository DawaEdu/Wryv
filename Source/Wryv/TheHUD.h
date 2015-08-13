#pragma once

#include <vector>
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
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* MapSlotEntryBackgroundTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* StackPanelTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* TooltipBackgroundTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* SolidWhiteTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* PauseButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* ResumeButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* BuildButtonTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* NullTexture;
  
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterialInstance* ClockMaterialInstance;
  // The blueprint for the fog of war instance to use.
  
  // Render-to-texture target. Created inside the editor.
  USceneCaptureComponent2D *rendererIcon, *rendererMinimap;

  // Because canvas has to be valid for box selection to work it seems
  bool WillSelectNextFrame, SelectAdds;

  // The buttons currently showing on the user interface.
  UserInterface* ui; // The root UI widget. It doesn't have a viz, but it parents all other display containers.
  
  bool Init;  // Global init for all objects
  FBox2D selectBox;

  // The uClass of the selected object highlight
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass* uClassSelector;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass* uClassSelectorA;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass* uClassSelectorShop;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D* PortraitTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D* MinimapTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* MediaTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterialInterface* MediaMaterial;
  //UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMediaPlayer* mediaPlayer;

  // The font used to render the text in the HUD.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *smallFont;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *largeFont;

  Types NextSpell;  // The next spell to be cast by the UI, 0 if no spell is queued & ready
  Types NextBuilding;  // NULL if no building is trying to be placed.

  // This is the ring shaped selector that gets attached to the currently selected unit(s)
  vector<AWidget3D*> selectors, selAttackTargets;
  AWidget3D *selectorShopPatron;

  // Make a texture for rendering the fog of war to
  UCanvasRenderTarget2D* RTFogOfWar; // : UTexture

  // The blot in the material to use as a fogofwar blot
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UMaterial *WarBlot;

  // This is the currently displayed amount of gold,lumber,stone
  // These are state variables since they are refreshed each frame.
  float displayedGold, displayedLumber, displayedStone;
  
  ATheHUD(const FObjectInitializer& PCIP);
  virtual void BeginPlay() override;
  TArray<FAssetData> ScanFolder( FName folder );
  
  EventCode BeginBoxSelect( FVector2D mouse );
  EventCode Hover( FVector2D mouse );
  EventCode DragBoxSelect( FVector2D mouse );
  EventCode EndBoxSelect( FVector2D mouse );
  vector<AGameObject*> Pick( FBox2DU box );

  vector<AGameObject*> Select( vector<AGameObject*> objects );
  
  void Unselect( AGameObject* object );
  void UnselectAsTarget( AGameObject* object );
  EventCode TogglePause();

  void InitWidgets();
  void Setup();
  
  void UpdateDisplayedResources();
  void UpdateMouse();

  bool Valid() { return IsCanvasValid_WarnIfNot() ; }

  void DrawMaterial(UCanvas* Canvas, UMaterialInterface* Material, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float MaterialU, float MaterialV, float MaterialUWidth, float MaterialVHeight);
  void DrawMaterial(UCanvas* Canvas, UMaterialInterface* Material, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float MaterialU, float MaterialV, float MaterialUWidth, float MaterialVHeight, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot);
  void DrawTexture(UCanvas* Canvas, UTexture* Texture, float ScreenX, float ScreenY, float ScreenW, float ScreenH, float TextureU, float TextureV, float TextureUWidth, float TextureVHeight, FLinearColor TintColor=FLinearColor::White, EBlendMode BlendMode=BLEND_Translucent, float Scale=1.f, bool bScalePosition=false, float Rotation=0.f, FVector2D RotPivot=FVector2D::ZeroVector);

  UFUNCTION()
  void DrawFogOfWar(UCanvas* theCanvas, int32 Width, int32 Height);
  void DrawPortrait();
  virtual void DrawHUD() override;

  // Which widget was hit by the mouse
  HotSpot* MouseDownLeft( FVector2D mouse );
  HotSpot* MouseUpLeft( FVector2D mouse );
  HotSpot* MouseDownRight( FVector2D mouse );
  HotSpot* MouseUpRight( FVector2D mouse );
  
  HotSpot* MouseMoved( FVector2D mouse );
  
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
