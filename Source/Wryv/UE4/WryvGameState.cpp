#include "Wryv.h"
#include "Util/GlobalFunctions.h"
#include "UE4/WryvGameInstance.h"
#include "WryvGameState.h"

AWryvGameState::AWryvGameState(const FObjectInitializer& PCIP) : Super(PCIP)
{
  PrimaryActorTick.bCanEverTick = 1;
  LOG("AWryvGameState::AWryvGameState()");
}

void AWryvGameState::Tick( float t )
{
  
}
