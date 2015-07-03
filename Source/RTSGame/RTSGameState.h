#pragma once

#include "GameFramework/GameState.h"
#include "RTSGameState.generated.h"

UCLASS()
class RTSGAME_API ARTSGameState : public AGameState
{
  GENERATED_BODY()
public:
  ARTSGameState( const FObjectInitializer& PCIP );
  virtual void Tick( float t );
};


