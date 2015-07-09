#pragma once

#include "Building.h"
#include "Types.h"
#include "Array.h"
#include "ItemShop.generated.h"

class AUnit;

UCLASS()
class RTSGAME_API AItemShop : public ABuilding
{
	GENERATED_UCLASS_BODY()
public:
  // The nearest ItemShop patron.
  AUnit* patron;

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Prox )
  USphereComponent* sphere;

  // A listing of the items contained inside the shop
  TArray< TEnumAsByte< Types > > Inventory;

  virtual void BeginPlay() override;

  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void Prox( AActor* OtherActor, UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult );

  UFUNCTION(BlueprintNativeEvent, Category = Collision)
  void ProxEnd( AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex );

};
