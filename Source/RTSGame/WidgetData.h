#pragma once

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
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) UTexture2D *Icon;
  // Location of the texture, size, and text's position.
  FVector2D Pos;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FVector2D Size;
  FVector2D TextPos;
  // The color the text is drawn in.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FLinearColor TextColor;
  FString ToolTip;

  // I want to connect the widget with the unitsdatarow for it, which
  // has the cost for spawning the item.
  FWidgetData(){
    Icon = 0;
    Type = NOTHING;
    TextColor = FLinearColor::White;
  }
  float left(){ return Pos.X; }
	float right(){ return Pos.X + Size.X; }
	float top(){ return Pos.Y; }
	float bottom(){ return Pos.Y + Size.Y; }
	bool hit( FVector2D v )
	{
		// +---+ top (0)
		// |   |
		// +---+ bottom (2) (bottom > top)
		// L   R
		return v.X > left() && v.X < right() &&
           v.Y > top() && v.Y < bottom();
	}

  FString ToString();

};
