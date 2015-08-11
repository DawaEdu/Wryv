#include "Wryv.h"
#include "Resource.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

AResource::AResource( const FObjectInitializer& PCIP ) : AGameObject(PCIP)
{
}

void AResource::BeginPlay()
{
  Super::BeginPlay();
}

