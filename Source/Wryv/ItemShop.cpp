#include "Wryv.h"
#include "ItemShop.h"
#include "WryvGameInstance.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Unit.h"

AItemShop::AItemShop( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  patron = 0;
}

void AItemShop::BeginPlay()
{
  Super::BeginPlay();
}

