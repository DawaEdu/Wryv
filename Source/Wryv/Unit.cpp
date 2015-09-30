#include "Wryv.h"
#include "Unit.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "PlayerControl.h"
#include "TheHUD.h"
#include "Widget.h"
#include "UnitsData.h"

#include <map>
using namespace std;

// Sets default values
AUnit::AUnit( const FObjectInitializer& PCIP ) : AGameObject( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, "themeshof" );
  Mesh->AttachTo( DummyRoot );
}

void AUnit::BeginPlay()
{
  Super::BeginPlay();
}

void AUnit::Move( float t )
{
  // recompute path
  if( Stats.SpeedMax )
  {
    // Prioritize the FollowTarget.
    if( FollowTarget && AttackTarget )
    {
      warning( FS( "%s had both FollowTarget=%s AttackTarget=%s",
        *Stats.Name, *FollowTarget->Stats.Name, *AttackTarget->Stats.Name ) );
    }

    if( FollowTarget )
    {
      MoveWithinDistanceOf( FollowTarget,  FollowTarget->Radius() );
    }
    else if( AttackTarget )
    {
      MoveWithinDistanceOf( AttackTarget, Stats.AttackRange * 0.9f );
      Face( AttackTarget->Pos );
    }
  }

  Walk( t );   // Walk towards destination
  AGameObject::Move( t );
}


