#pragma once

#include "Game/Enums.h"
#include "SoundEffect.generated.h"

USTRUCT()
struct WRYV_API FSoundEffect
{
  GENERATED_USTRUCT_BODY()

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
  TEnumAsByte<UISounds> Type;
  
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
  USoundBase* Sound;

  FSoundEffect():Type((UISounds)0),Sound(0)
  {
  }
};
