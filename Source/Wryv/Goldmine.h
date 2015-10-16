#pragma once

#include "ExplodableResource.h"
#include "Goldmine.generated.h"

UCLASS()
class WRYV_API AGoldmine : public AExplodableResource
{
	GENERATED_UCLASS_BODY()
public:
  //AGoldmine(const FObjectInitializer& PCIP);
  virtual UClass* GetCPPClass() { return StaticClass(); }

};


