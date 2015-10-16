#pragma once

#include "GameObject.h"
#include "Item.generated.h"

class UItemAction;
class AUnit;

UCLASS()
class WRYV_API AItem : public AGameObject
{
  GENERATED_UCLASS_BODY()
public:
  // The small portrait of the item when used as a buff on a unit.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  UTexture* BuffPortrait;
  // When the item is picked up, this is the class of the button.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  TSubclassOf<UItemAction> ButtonClass;
  // Mesh for the item.
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  UStaticMeshComponent* Mesh;
  // Quantity of item in slot
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Properties )  int32 Quantity;

  virtual void Move( float t ) override;
  virtual void Hit( AGameObject* other ) override;
  virtual void Die() override;
  //Usage from UItemAction (ButtonClass), not from Item class directly.
  //void Use( AUnit* unit );
};
