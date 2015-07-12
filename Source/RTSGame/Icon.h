#pragma once

#include "Types.h"
#include "GameFramework/Actor.h"
#include "Icon.generated.h"

/// Contains all the data for an Icon object.
/// 
USTRUCT()
struct RTSGAME_API FIcon
{
	GENERATED_USTRUCT_BODY()
public:
  // Type of object this widget represents. The widget spawns a unit of this type.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) TEnumAsByte<Types> Type;
  // The label that appears on this widget.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FString Label;
  // the texture this widget uses, which is different from the FUnitsDataRow pic
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) UTexture* Icon;
  // Location of the texture, size, and text's position.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FVector2D Size;

};


