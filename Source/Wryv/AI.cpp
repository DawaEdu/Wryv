#pragma once
#include "Wryv.h"

#include "AI.h"
#include "Team.h"
#include "Types.h"

FAI::FAI()
{
  aiLevel = AILevel::AINone;
  foodFraction = 2.f/3.f; // default of 66%
  timeSinceLastScout = 0.f;
  scoutInterval = 20.f;
  
  // Try to keep these stock amounts of each resource type
  StartingGold = 1000;
  StartingLumber = 500;
  StartingStone = 50;
}

vector<Types> FAI::GetNeededResourceTypes( Team& team )
{
  vector<Types> types;

  // recommend to get the resource type of lowest % of starting amounts.
  float goldP = (float)team.Gold / StartingGold;
  float lumberP = (float)team.Lumber / StartingLumber;
  float stoneP = (float)team.Stone / StartingStone;

  // which has the greatest need
  if( goldP < lumberP   &&   goldP < stoneP )
  {
    if( lumberP < stoneP )
      types = { RESGOLD, RESLUMBER, RESSTONE };
    else
      types = { RESGOLD, RESSTONE, RESLUMBER };
  }
  else if( lumberP < goldP   &&   lumberP < stoneP )
  {
    if( goldP < stoneP )
      types = { RESLUMBER, RESGOLD, RESSTONE };
    else
      types = { RESLUMBER, RESSTONE, RESGOLD };
  }
  else // stone is scarecest
  {
    if( lumberP < goldP )
      types = { RESSTONE, RESLUMBER, RESGOLD };
    else
      types = { RESSTONE, RESGOLD, RESLUMBER }; // EQUAL
  }


  info( "Types needed are " );
  for( Types t : types )
    info( FS( "  * %s", *GetTypesName( t ) ) );

  return types;
}

FString FAI::ToString()
{
  return FS( "AILevel aiLevel=%d foodFraction=%f scoutInterval=%f gold=%d lumber=%d stone=%d", 
    (int)aiLevel.GetValue(), foodFraction, scoutInterval,
    StartingGold, StartingLumber, StartingStone );
}


