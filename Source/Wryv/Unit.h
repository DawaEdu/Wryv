#pragma once

#include <vector>
#include <set>
using namespace std;

#include "GameObject.h"
#include "ItemActionClassAndQuantity.h"
#include "Unit.generated.h"

class AItem;

class UItemAction;
class UUnitAction;


UCLASS()
class WRYV_API AUnit : public AGameObject
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cosmetics)
  USkeletalMeshComponent* Mesh;
  
  // The abilities this unit has, including ability to build, etc.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TArray< TSubclassOf< UUnitAction > > Abilities;
  // COOLDOWNS: A list of these for each of our capabilities in 
  UPROPERTY() TArray< UUnitAction* > CountersAbility;

  // The items the unit starts carrying
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TArray< FItemActionClassAndQuantity > StartingItems;
  UPROPERTY() TArray< UItemAction* > CountersItems;
  // Items unit is holding in-play.
  // The cooldown time remaining for item by class type
  // is stored here, by unit.
  
  // Weapon properties: If attacks send a projectile, set object here.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Capabilities)
  TSubclassOf< AProjectile > ReleasedProjectileWeapon;

  UFUNCTION(BlueprintCallable, Category = Fighting)  virtual void AttackCycle();
  
  AUnit(const FObjectInitializer& PCIP);
  virtual void InitIcons();
  void AddItem( FItemActionClassAndQuantity itemQuantity );
  virtual void PostInitializeComponents() override;
  virtual void BeginPlay() override;
  bool UseAbility( int index );
  bool UseItem( int index );
  virtual void MoveCounters( float t ) override;
  virtual void OnUnselected();

  void Shoot();

};
