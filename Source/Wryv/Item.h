#pragma once

#include "GameObject.h"
#include "Item.generated.h"

UCLASS()
class WRYV_API AItem : public AGameObject
{
  GENERATED_UCLASS_BODY()
public:
  virtual void BeginPlay() override;

  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void ProxPickup( AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult );

};
