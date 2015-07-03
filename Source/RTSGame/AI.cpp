#pragma once
#include "RTSGame.h"

#include "AI.h"
#include "Team.h"

Types AI::GetNeededResourceType( Team& team )
{
  // recommend to get the resource type of lowest %
  float goldP = (float)team.Gold / StockGold;
  float lumberP = (float)team.Lumber / StockLumber;
  float stoneP = (float)team.Stone / StockStone;

  // which has the greatest need
  if( goldP < lumberP   &&   goldP < stoneP )
  {
    return Types::RESGOLDMINE;
  }
  else if( lumberP < goldP   &&   lumberP < stoneP )
  {
    return Types::RESTREEEVERGREEN;
  }
  else
  {
    return Types::RESSTONE;
  }

}
