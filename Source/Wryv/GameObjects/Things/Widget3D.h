#pragma once

#include "GameObjects/Things/Shape.h"
#include "Widget3D.generated.h"

/// <A> These are just 3D onscreen widgets like Selectors etc.
UCLASS()
class WRYV_API AWidget3D : public AShape
{
  GENERATED_BODY()
public:
  AWidget3D( const FObjectInitializer& PCIP );
  virtual void BeginPlay() override;
  virtual void Tick( float t ) override;
};
