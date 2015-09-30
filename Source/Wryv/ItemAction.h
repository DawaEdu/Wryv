#pragma once

#include "Action.h"
#include "ItemAction.generated.h"

class AGameObject;
class AItem;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A motion action") )
class WRYV_API UItemAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf< AItem > Item;
  // The icon applied (if any) when this item is applied as a buff to a unit
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  UTexture* BuffIcon;
  // How many of this item player has
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  int32 Quantity;

  virtual void Go(AGameObject* go) override;
  virtual void OnComplete() override;
  
};
