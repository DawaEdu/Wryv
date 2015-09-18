#pragma once

#include "Shape.h"
#include "Widget3D.generated.h"

UCLASS()
class WRYV_API AWidget3D : public AShape
{
  GENERATED_UCLASS_BODY()
public:
  virtual void BeginPlay() override;
  virtual void Tick( float t ) override;
};
