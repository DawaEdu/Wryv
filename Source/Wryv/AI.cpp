#pragma once
#include "Wryv.h"

#include "AI.h"
#include "Goldmine.h"
#include "Team.h"
#include "Resource.h"
#include "Stone.h"
#include "Tree.h"

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

vector< TSubclassOf<AResource> > FAI::GetNeededResourceTypes( Team& team )
{
  vector< TSubclassOf<AResource> > types;

  // recommend to get the resource type of lowest % of starting amounts.
  float goldP = (float)team.Gold / StartingGold;
  float lumberP = (float)team.Lumber / StartingLumber;
  float stoneP = (float)team.Stone / StartingStone;

  // which has the greatest need
  if( goldP < lumberP   &&   goldP < stoneP )
  {
    if( lumberP < stoneP )
      types = { AGoldmine::StaticClass(), ATree::StaticClass(), AStone::StaticClass() };
    else
      types = { AGoldmine::StaticClass(), AStone::StaticClass(), ATree::StaticClass() };
  }
  else if( lumberP < goldP   &&   lumberP < stoneP )
  {
    if( goldP < stoneP )
      types = { ATree::StaticClass(), AGoldmine::StaticClass(), AStone::StaticClass() };
    else
      types = { ATree::StaticClass(), AStone::StaticClass(), AGoldmine::StaticClass() };
  }
  else // stone is scarecest
  {
    if( lumberP < goldP )
      types = { AStone::StaticClass(), ATree::StaticClass(), AGoldmine::StaticClass() };
    else
      types = { AStone::StaticClass(), AGoldmine::StaticClass(), ATree::StaticClass() }; // EQUAL
  }

  return types;
}

FString FAI::ToString()
{
  return FS( "AILevel aiLevel=%d foodFraction=%f scoutInterval=%f gold=%d lumber=%d stone=%d", 
    (int)aiLevel.GetValue(), foodFraction, scoutInterval,
    StartingGold, StartingLumber, StartingStone );
}


