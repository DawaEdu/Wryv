#pragma once

#include <vector>
#include <map>
using namespace std;

#include "UnitsData.h"
#include "WidgetData.h"
#include "UnitTypeUClassPair.h"
#include "Team.h"
#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

class APlayerControl;
class AFlyCam;
class AGameObject;

UCLASS()
class RTSGAME_API AMyHUD : public AHUD
{
  GENERATED_BODY()
public:
  // These are the widgets for the UI icons:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FWidgetData widgetMouseCursorHand;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FWidgetData widgetMouseCursorCrossHairs;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FWidgetData widgetGold;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FWidgetData widgetLumber;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FWidgetData widgetStone;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FWidgetData widgetBkg;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FWidgetData widgetIcon; //lastclickobject widget
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FWidgetData widgetMinimap; //widget for the minimap
  
  // The uClass of the selected object highlight
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass *uClassSelector;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass *uClassSelectorA;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass *uClassSelectorShop;

  // DO NOT USE this TArray, use Game->unitsData.uClass instead.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) TArray< FUnitTypeUClassPair > UnitTypeUClasses;

  // Render-to-texture target. Created inside the editor.
  USceneCaptureComponent2D *rendererIcon, *rendererMinimap;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D *texIcon;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTextureRenderTarget2D *texMinimap;

  // The texture etc to use for last clicked object
  AGameObject *lastClickedObject;

  // Manually code connected
  map<Types,FWidgetData> widgets;
  // widgets active this frame (discards each frame).
  vector<FWidgetData> frameWidgets;
  FWidgetData* hoverWidget; // The widget over which the mouse is hovering

  // The next spell to be cast by the UI, 0 if no spell is queued & ready
  Types NextSpell;
  // NULL if no building is trying to be placed.
  Types NextBuilding;

  // This is the ring shaped selector that gets
  // attached to the currently selected unit
  AActor *selector, *selectorAttackTarget, *selectorShopPatron;
  
  FString statusMsg;
  static float BarSize;
  static float Margin;
  static FVector2D IconSize;

  // This is the currently displayed amount of gold,lumber,stone
  float displayedGold, displayedLumber, displayedStone;
	
  // The font used to render the text in the HUD.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *hudFont;

  AMyHUD(const FObjectInitializer& PCIP);

  // Get the Z distance that an object should be placed
  // when it has a world radius of radiusWorldUnits
  // such that it has a rendered width of radiusOnScreenPx
  // on a texture of width texW when the fov is fovyDegrees
  // texW is required to compute clipSpace coordinates,
  // which is the coordinate system that the formula works in.
  float GetZDistance( float radiusWorldUnits, float radiusOnScreenPx,
    float texW, float fovyDegrees );

  // Compute the resultant pixel width of an object:
  //   - with radius radiusWorldUnits
  //   - at a distance distanceToObject units away from the camera
  // where the texture has width=texW and the camera
  // has FOV=fovyDegrees
  float GetPxWidth( float radiusWorldUnits, float distanceToObject, float texW, float fovyDegrees );
  void RenderScreen( USceneCaptureComponent2D* renderer, UTextureRenderTarget2D* tt, FVector objectPos, float radiusWorldUnits, FVector cameraDir );
  void Setup();
  float DrawCost( Types res, float x, float size, int cost );
  void DrawWidget( FWidgetData& bw );
  void DrawGroup( TArray< TEnumAsByte<Types> >& v, float xPos, float yPos, float size, float margin, bool horizontalStack, bool label, bool purchaseables );
  void SetAttackTargetSelector( AGameObject* target );
  void SetShopTargetSelector( AGameObject* target );
  void DrawSidebar();
  void DrawTooltipHover();
  void UpdateDisplayedResources();
  void DrawInfoBars();
  virtual void DrawHUD() override;
  void DrawMouseCursor();
  bool Purchase( Types itemType );
  void RunEvent( Types buttonType );

  // Which widget was hit by the mouse
  bool MouseClicked( FVector2D mouse );
  bool MouseMoved( FVector2D mouse );
  FString DrawToolTipBuyCost( FUnitsDataRow& ud );
  FString DrawToolTipUseCost( FUnitsDataRow& ud );
  virtual void Tick( float t );
};
