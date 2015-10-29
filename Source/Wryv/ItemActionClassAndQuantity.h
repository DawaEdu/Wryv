#pragma once

#include "Wryv.h"
#include "ItemActionClassAndQuantity.generated.h"

class UItemAction;

USTRUCT()
struct WRYV_API FItemActionClassAndQuantity
{
  GENERATED_USTRUCT_BODY()
  // We indicate the ItemAction instead of the Item because we can get
  // the Item Class type from UItemAction, but not the other way around.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  TSubclassOf< UItemAction > ItemActionClass;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  int32 Quantity;
  FItemActionClassAndQuantity()
  {
    ItemActionClass = 0;
    Quantity = 1;
  }
};


