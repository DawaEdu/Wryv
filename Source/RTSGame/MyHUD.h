#pragma once

#include <vector>
#include <map>
using namespace std;

#include "Icon.h"
#include "IconGroup.h"
#include "Widget.h"
#include "UnitsData.h"
#include "WidgetData.h"
#include "UnitTypeUClassPair.h"
#include "Team.h"
#include "CursorTexture.h"
#include "GameFramework/HUD.h"

#include "MyHUD.generated.h"

class APlayerControl;
class AFlyCam;
class AGameObject;
struct SlotPalette;
struct Tooltip;
struct CostWidget;
struct ImageWidget;
struct Panel;
struct StackPanel;
struct UserInterface;

UCLASS()
class RTSGAME_API AMyHUD : public AHUD
{
  GENERATED_BODY()
public:
  // These are the widgets for the UI icons:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FCursorTexture MouseCursorHand;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FCursorTexture MouseCursorCrossHairs;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* GoldIconTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* LumberIconTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* StoneIconTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* RightPanelTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* SlotPaletteTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* TooltipBackgroundTexture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* SolidWhiteTexture;
  
  // Render-to-texture target. Created inside the editor.
  USceneCaptureComponent2D *rendererIcon, *rendererMinimap;
  
  // The buttons currently showing on the user interface.
  UserInterface* ui; // The root UI widget. It doesn't have a viz, but it parents all other display containers.
  
  //vector<HotSpot*> buttons;
  bool Init;  // Global init for all objects
  FBox2D selectBox;

  // Maps UNITTYPE=>WIDGET for it. Connected by code at game-startup, since
  // Widget faces are specified in the Blueprints for each individual model
  map<Types,FWidgetData> widgets;
  
  // The uClass of the selected object highlight
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass *uClassSelector;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass *uClassSelectorA;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass *uClassSelectorShop;
  // DO NOT USE this TArray, use Game->unitsData.uClass instead.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) TArray< FUnitTypeUClassPair > UnitTypeUClasses;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D *texIcon;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D *texMinimap;
  
  // The font used to render the text in the HUD.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *hudFont;

  // The texture etc to use for last clicked object
  AGameObject *SelectedObject;
  // The next spell to be cast by the UI, 0 if no spell is queued & ready
  Types NextSpell;
  // NULL if no building is trying to be placed.
  Types NextBuilding;

  // This is the ring shaped selector that gets
  // attached to the currently selected unit
  AActor *selector, *selectorAttackTarget, *selectorShopPatron;
  FString statusMsg;
  
  // This is the currently displayed amount of gold,lumber,stone
  // These are state variables since they are refreshed each frame.
  float displayedGold, displayedLumber, displayedStone;
  
  AMyHUD(const FObjectInitializer& PCIP);
  virtual void BeginDestroy() override;
  void InitWidgets();
  void Setup();
  void SetAttackTargetSelector( AGameObject* target );
  void SetShopTargetSelector( AGameObject* target );
  
  void UpdateSelectedObjectStats();
  void UpdateDisplayedResources();
  void UpdateMouse();
  virtual void DrawHUD() override;
  bool Purchase( Types itemType );
  void RunEvent( Types buttonType );

  // Which widget was hit by the mouse
  bool MouseLeftDown( FVector2D mouse );
  bool MouseLeftUp( FVector2D mouse );
  bool MouseRightDown( FVector2D mouse );
  bool MouseRightUp( FVector2D mouse );
  
  bool MouseDragged( FVector2D mouse );
  bool MouseHovered( FVector2D mouse );
  bool MouseMoved( FVector2D mouse );

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
};
