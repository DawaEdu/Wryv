#pragma once

#include <vector>
using namespace std;

#include "Types.h"
#include "GameFramework/Actor.h"
#include "GameObject.h"
#include "ItemQuantity.h"
#include "Unit.generated.h"

UCLASS()
class WRYV_API AUnit : public AGameObject
{
	GENERATED_UCLASS_BODY()
public:
  // These start with # items in editor. Defaults "Nothing" in toolbelt
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=UnitProperties)  TArray< TEnumAsByte< Types > > StartingItems;
  // The items the unit is holding
  TArray< FUnitsDataRow > Items;
  //AUnit(const FObjectInitializer& PCIP);
  virtual void BeginPlay() override;
  void ConsumeItem( int i );
  // Function that runs whenever the unit is first clicked on or selected.
  virtual void Move( float t ) override;
};
