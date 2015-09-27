#include "Wryv.h"
#include "PlayerStats.h"

UPlayerStats::UPlayerStats( const FObjectInitializer& PCIP ) : Super( PCIP )
{
}

FString UPlayerStats::ToString()
{
  return FString::Printf( TEXT("AILevel %d"), aiLevel );
}

