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
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) UTexture *Icon;
  // Location of the texture, size, and text's position.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) FVector2D Size;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData) int32 ShortcutKey;
  
  // The color the text is drawn in.
  FString ToolTip;

  // PurchaseButtons cost money, while use buttons trigger the item's use.
  //bool isPurchaseButton;

  // I want to connect the widget with the unitsdatarow for it, which
  // has the cost for spawning the item.
  FWidgetData(){
    Icon = 0;
  }
  
  //float left(){ return Pos.X; }
	//float right(){ return Pos.X + Size.X; }
	//float top(){ return Pos.Y; }
	//float bottom(){ return Pos.Y + Size.Y; }
	//bool hit( FVector2D v )
	//{
	//	// +---+ top (0)
	//	// |   |
	//	// +---+ bottom (2) (bottom > top)
	//	// L   R
	//	return v.X > left() && v.X < right() &&
  //         v.Y > top() && v.Y < bottom();
	//}
  //FVector2D getHitPercent( FVector2D v ) { return (v - Pos)/Size; }
};

