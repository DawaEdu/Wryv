#pragma once

#include "Action.h"
#include "ItemActionClassAndQuantity.h"
#include "ItemAction.generated.h"

class AGameObject;
class AItem;
class AUnit;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A motion action") )
class WRYV_API UItemAction : public UAction
{
  GENERATED_BODY()
public:
  // Class of the item itself
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  TSubclassOf< AItem > ItemClass;
  // How many of this item player has
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  int32 Quantity;
  // The icon applied (if any) when this item is applied as a buff to a unit
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UTexture* BuffIcon;
  UPROPERTY() AUnit* AssociatedUnit; // Revise that this is set correctly everywhere in the code.
  UPROPERTY() FString AssociatedUnitName;

  UItemAction( const FObjectInitializer & PCIP );
  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime() override;
  virtual bool Click();
  virtual void Step( float t );
  void PopulateClock( Clock* inClock, int i );

};
