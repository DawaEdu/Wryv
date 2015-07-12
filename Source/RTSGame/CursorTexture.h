#pragma once

#include "RTSGame.h"
#include "CursorTexture.generated.h"

USTRUCT()
struct RTSGAME_API FCursorTexture
{
	GENERATED_USTRUCT_BODY()
public:
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) UTexture* Texture;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = HUD ) FVector2D Hotpoint;
};

