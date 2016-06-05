#include "Wryv.h"
#include "AIProfile.h"

UAIProfile::UAIProfile( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  
}

FString UAIProfile::ToString()
{
  return ai.ToString();
}


