#pragma once

#include "Types.h"

struct Team;

// The artificial intelligence for the team.
// Parameters for the AI
struct AI
{
  // The AI level being used. 0 is no ai (for player-controlled)
  // higher levels mean higher ai (greater difficulty)
  int level;

  // How much spare food do we want to keep in stock?
  // If food% is 40/60 (66%) then perhaps we want to build another farm.
  float foodPercentage;

  // This is the next building this team wants to build.
  // If the resources become available to build this type of building
  // then the Team builds it with a peasant unit.
  //Types NextBuilding;

  // Attacks are periodic. We scout at intervals.
  float scoutInterval, timeSinceLastScout;

  // Need some defaults for stock Gold, Lumber, Stone AI level expects to have
  int32 StockGold, StockStone, StockLumber;
  
  AI()
  {
    level = 0;
    foodPercentage = 2.f/3.f; // default of 66%
    timeSinceLastScout = 0.f;
    scoutInterval = 20.f;

    // Try to keep these stock amounts of each resource type
    StockGold = 1000;
    StockLumber = 500;
    StockStone = 50;
    //NextBuilding = NOTHING;
  }
  
  Types GetNeededResourceType( Team& team );
};


