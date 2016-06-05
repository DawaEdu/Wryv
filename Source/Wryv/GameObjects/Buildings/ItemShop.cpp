#include "Wryv.h"
#include "ItemShop.h"
#include "UE4/WryvGameInstance.h"
#include "GameObjects/GameObject.h"
#include "Util/GlobalFunctions.h"
#include "GameObjects/Units/Unit.h"

AItemShop::AItemShop( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  patron = 0;
}

void AItemShop::BeginPlay()
{
  Super::BeginPlay();
}

