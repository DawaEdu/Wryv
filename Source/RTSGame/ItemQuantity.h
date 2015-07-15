#pragma once

#include "RTSGame.h"
#include "Types.h"
#include "ItemQuantity.generated.h"

USTRUCT()
struct RTSGAME_API FItemQuantity
{
  GENERATED_USTRUCT_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) TEnumAsByte<Types> Type;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) int32 Quantity;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData) float Cooldown;
  FItemQuantity():Type(Types::NOTHING),Quantity(1),Cooldown(1.f){}
  FItemQuantity( Types type, int32 quantity ) : Type( type ), Quantity( quantity ) { }
};
