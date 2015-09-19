#include "Wryv.h"
#include "GlobalFunctions.h"
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

