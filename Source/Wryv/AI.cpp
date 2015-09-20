#pragma once
#include "Wryv.h"

#include "AI.h"
#include "Team.h"
#include "Types.h"

//FAI::FAI( const FObjectInitializer& PCIP ) : Super( PCIP ){ }
//Types FAI::GetNeededResourceType( Team& team )
//{
//  // recommend to get the resource type of lowest %
//  float goldP = (float)team.Gold / StartingGold;
//  float lumberP = (float)team.Lumber / StartingLumber;
//  float stoneP = (float)team.Stone / StartingStone;
//
//  // which has the greatest need
//  if( goldP < lumberP   &&   goldP < stoneP )
//  {
//    return Types::RESGOLD;
//  }
//  else if( lumberP < goldP   &&   lumberP < stoneP )
//  {
//    return Types::RESLUMBER;
//  }
//  else
//  {
//    return Types::RESSTONE;
//  }
//
//}
