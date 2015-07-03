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
  
  // The uClass of the selected object highlight
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass *uClassSelector;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UClass *uClassSelectorA;

  // DO NOT USE this TArray, use Game->unitsData.uClass instead.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD )
  TArray< FUnitTypeUClassPair > UnitTypeUClasses;

  // The texture etc to use for last clicked object
  AGameObject *lastClickedObject;

  // Manually code connected
  map<Types,FWidgetData> widgets;
  // widgets active this frame (discards each frame).
  vector<FWidgetData> frameWidgets;
  
  // The next spell to be cast by the UI, 0 if no spell is queued & ready
  Types NextSpell;
  // NULL if no building is trying to be placed.
  Types NextBuilding;

  // This is the ring shaped selector that gets
  // attached to the currently selected unit
  AActor *selector, *selectorAttackTarget;
  
  FString statusMsg;
  static float margin;
  static FVector2D IconSize;

  // This is the currently displayed amount of gold,lumber,stone
  float displayedGold, displayedLumber, displayedStone;
	
  // The font used to render the text in the HUD.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UFont *hudFont;

  AMyHUD(const FObjectInitializer& PCIP);

  void Setup();
  void DrawWidget( FWidgetData& bw );
  void DrawGroup( TArray< TEnumAsByte<Types> >& v, float xPos, float yPos, float size, float margin, bool horizontalStack, bool label );
  void DrawSidebar();

  void UpdateDisplayedResources();
  void DrawTopbar();
  virtual void DrawHUD() override;
  void RunEvent( Types buttonType );

  // Which widget was hit by the mouse
  bool MouseClicked( FVector2D mouse );
  bool MouseMoved( FVector2D mouse );
  FString GetToolTip( FUnitsDataRow& ud );
  FString GetToolTip( AGameObject* go );
  virtual void Tick( float t );
};
