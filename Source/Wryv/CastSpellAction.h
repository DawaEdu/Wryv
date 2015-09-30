#pragma once

#include "Action.h"
#include "CastSpellAction.generated.h"

class AGameObject;
class AProjectile;

UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="Action for casting a spell") )
class WRYV_API UCastSpellAction : public UAction
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  TSubclassOf< AProjectile > Projectile;

  virtual void Go(AGameObject* go);
  virtual void OnComplete();
};
