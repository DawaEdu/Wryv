#include "Wryv.h"
#include "WryvGameState.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

AWryvGameState::AWryvGameState(const FObjectInitializer& PCIP) : Super(PCIP)
{
  PrimaryActorTick.bCanEverTick = true;
  LOG("AWryvGameState::AWryvGameState()");
}

void AWryvGameState::Tick( float t )
{
  
}
