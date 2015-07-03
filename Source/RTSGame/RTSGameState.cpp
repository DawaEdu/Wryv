#include "RTSGame.h"
#include "RTSGameState.h"
#include "RTSGameInstance.h"

ARTSGameState::ARTSGameState(const FObjectInitializer& PCIP) : Super(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
  UE_LOG( LogTemp, Warning, TEXT("ARTSGameState::ARTSGameState()") );
}

void ARTSGameState::Tick( float t )
{
  Game->tick++;
}
