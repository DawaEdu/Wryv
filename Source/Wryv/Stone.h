#pragma once

#include "ExplodableResource.h"
#include "Stone.generated.h"

UCLASS()
class WRYV_API AStone : public AExplodableResource
{
	GENERATED_UCLASS_BODY()
public:
  //AStone(const FObjectInitializer& PCIP);
  virtual UClass* GetCPPClass() { return StaticClass(); }

};


