#pragma once

#include "Action.h"
#include "ItemAction.generated.h"

class AGameObject;
class AItem;
class AUnit;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="A motion action") )
class WRYV_API UItemAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  TSubclassOf< AItem > ItemClass;
  // The icon applied (if any) when this item is applied as a buff to a unit
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UTexture* BuffIcon;
  // How many of this item player has
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  int32 Quantity;
  UPROPERTY() AUnit* AssociatedUnit; // Revise that this is set correctly everywhere in the code.
  UPROPERTY() FString AssociatedUnitName;

  virtual UTexture* GetIcon() override;
  virtual float GetCooldownTotalTime() override;
  virtual bool Click();
  virtual void Step( float t );
  void PopulateClock( Clock* inClock, int i );

};
