#pragma once

#include <functional>
#include <vector>
using namespace std;

#include "RTSGame.h"
#include "Types.h"
#include "GameFramework/Actor.h"
#include "WidgetData.generated.h"

struct FUnitsDataRow;

USTRUCT()
struct RTSGAME_API FWidgetData
{
	GENERATED_USTRUCT_BODY()
public:
  // Type of object this widget represents. The widget spawns a unit of this type.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) TEnumAsByte<Types> Type;
  // The label that appears on this widget.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FString Label;
  // the texture this widget uses, which is different from the FUnitsDataRow pic
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) UTexture *Tex;
  // Location of the texture, size, and text's position.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FVector2D Size;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FKey ShortcutKey; // autopopulates dialog with available keys
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FString Tooltip;
};

