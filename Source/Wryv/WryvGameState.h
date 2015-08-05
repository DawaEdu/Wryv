#pragma once

#include "GameFramework/GameState.h"
#include "WryvGameState.generated.h"

UCLASS()
class WRYV_API AWryvGameState : public AGameState
{
  GENERATED_BODY()
public:
  AWryvGameState( const FObjectInitializer& PCIP );
  virtual void Tick( float t );
};


