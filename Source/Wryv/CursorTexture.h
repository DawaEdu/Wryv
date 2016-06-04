#pragma once

#include "Wryv.h"
#include "CursorTexture.generated.h"

USTRUCT()
struct WRYV_API FCursorTexture
{
	GENERATED_USTRUCT_BODY()
public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD )
  UTexture* Texture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD )
  FVector2D Hotpoint;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD )
  FLinearColor Color;
};
