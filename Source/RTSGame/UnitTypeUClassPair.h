#pragma once

#include "RTSGame.h"
#include "Types.h"
#include "UnitTypeUClassPair.generated.h"

USTRUCT()
struct RTSGAME_API FUnitTypeUClassPair : public FTableRowBase
{
  GENERATED_USTRUCT_BODY()
public:
  // The UnitTYPE (strict set of types)
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  TEnumAsByte<Types> Type;
  // The UClass that should be instantiated for that Types
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitData)
  UClass *uClass;
};
