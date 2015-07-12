#pragma once

#include "Types.h"
#include "Icon.h"
#include "GameFramework/Actor.h"

#include "IconGroup.generated.h"

/// Contains all objects' icons
UCLASS()
class RTSGAME_API AIconGroup : public AActor
{
	GENERATED_UCLASS_BODY()
public:
  // Type of object this widget represents. The widget spawns a unit of this type.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WidgetData)
  TArray<FIcon> Icons;
  
};


