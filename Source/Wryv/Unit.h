#pragma once

#include <vector>
#include <set>
using namespace std;

#include "GameFramework/Actor.h"

#include "GameObject.h"
#include "Unit.generated.h"

class AItem;

UCLASS()
class WRYV_API AUnit : public AGameObject
{
	GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UnitProperties)  USkeletalMeshComponent* Mesh;

  //AUnit(const FObjectInitializer& PCIP);
  virtual void BeginPlay() override;
  
  // Function that runs whenever the unit is first clicked on or selected.
  virtual void Move( float t ) override;

};
