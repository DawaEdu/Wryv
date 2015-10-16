#include "Wryv.h"

#include "Building.h"
#include "GlobalFunctions.h"
#include "Team.h"
#include "Research.h"

#include "InProgressResearch.h"

UResearch::UResearch( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Text = "Unnamed research item";
  Building = 0;
  NextLevel = 0;
}

bool UResearch::Click()
{
  Building->UseResearch( UActionIndex );
  return 1;
}

