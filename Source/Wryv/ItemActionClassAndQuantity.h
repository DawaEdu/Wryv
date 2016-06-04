#pragma once

#include "Wryv.h"
#include "ItemActionClassAndQuantity.generated.h"

class UUIItemActionCommand;

USTRUCT()
struct WRYV_API FItemActionClassAndQuantity
{
  GENERATED_USTRUCT_BODY()
  // We indicate the ItemAction instead of the Item because we can get
  // the Item Class type from UUIItemActionCommand, but not the other way around.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )
  TSubclassOf< UUIItemActionCommand > ItemActionClass;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )
  int32 Quantity;
  FItemActionClassAndQuantity()
  {
    ItemActionClass = 0;
    Quantity = 1;
  }
};


