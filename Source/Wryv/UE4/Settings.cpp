#include "Wryv.h"
#include "Util/GlobalFunctions.h"
#include "Settings.h"

ASettings::ASettings( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  LOG( "ASettings::ASettings()" );
}

void ASettings::NotifyBeginPlay()
{
  Super::NotifyBeginPlay();
  LOG( "ASettings::NotifyBeginPlay()" );
}

