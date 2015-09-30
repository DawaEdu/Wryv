#pragma once

#include "GameObject.h"
#include "Item.generated.h"

class UItemAction;

UCLASS()
class WRYV_API AItem : public AGameObject
{
  GENERATED_UCLASS_BODY()
public:
  // The small portrait of the item when used as a buff on a unit.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Picture )
  UTexture* BuffPortrait;
  // When the item is picked up, this is the class of the button.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Picture )
  TSubclassOf<UItemAction> ButtonClass;

  virtual void Hit( AGameObject* other ) override;

};
