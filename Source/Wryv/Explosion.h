#pragma once

#include "GameObject.h"
#include "Explosion.generated.h"

UCLASS()
class WRYV_API AExplosion : public AGameObject
{
	GENERATED_UCLASS_BODY()
  //AExplosion(const FObjectInitializer& PCIP);
  virtual void Move( float t ) override;
  
};
