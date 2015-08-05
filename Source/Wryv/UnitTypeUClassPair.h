#pragma once

#include "Wryv.h"
#include "Types.h"
#include "UnitTypeUClassPair.generated.h"

USTRUCT()
struct WRYV_API FUnitTypeUClassPair
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
