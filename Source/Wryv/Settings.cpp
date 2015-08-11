#include "Wryv.h"
#include "Settings.h"
#include "GlobalFunctions.h"

ASettings::ASettings( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  LOG( "ASettings::ASettings()" );
}

void ASettings::NotifyBeginPlay()
{
  LOG( "ASettings::BeginPlay()" );
}

