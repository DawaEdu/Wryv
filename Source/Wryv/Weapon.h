#pragma once

#include "GameObject.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class WRYV_API AWeapon : public AGameObject
{
	GENERATED_UCLASS_BODY()
public:	
	virtual void BeginPlay() override;
	virtual void Tick( float t ) override;
};
