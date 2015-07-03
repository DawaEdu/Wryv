#include "RTSGame.h"
#include "Unit.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "PlayerControl.h"
#include "MyHUD.h"

#include <map>
using namespace std;

// Sets default values
AUnit::AUnit( const FObjectInitializer& PCIP ) : AGameObject( PCIP )
{
 	// Set this actor to call Tick() every frame.
  // You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  attackTarget = 0;
}


