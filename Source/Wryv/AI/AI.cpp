#pragma once
#include "Wryv.h"

#include "AI/AI.h"
#include "Game/Team.h"
#include "GameObjects/Things/Resources/Goldmine.h"
#include "GameObjects/Things/Resources/Resource.h"
#include "GameObjects/Things/Resources/Stone.h"
#include "GameObjects/Things/Resources/Tree.h"

FAI::FAI()
{
  aiLevel = AILevel::AINone;
  foodFraction = 2.f/3.f; // default of 66%
  timeSinceLastScout = 0.f;
  scoutInterval = 20.f;
  
  // Try to keep these stock amounts of each resource type
  StartingResources.Gold = 1000;
  StartingResources.Lumber = 500;
  StartingResources.Stone = 50;
}

vector< TSubclassOf<AResource> > FAI::GetNeededResourceTypes( Team& team )
{
  vector< TSubclassOf<AResource> > types;

  // recommend to get the resource type of lowest % of starting amounts.
  FCost percentages = team.Resources / StartingResources;
  
  // which has the greatest need
  if( percentages.Gold < percentages.Lumber   &&   percentages.Gold < percentages.Stone )
  {
    if( percentages.Lumber < percentages.Stone )
      types = { AGoldmine::StaticClass(), ATree::StaticClass(), AStone::StaticClass() };
    else
      types = { AGoldmine::StaticClass(), AStone::StaticClass(), ATree::StaticClass() };
  }
  else if( percentages.Lumber < percentages.Gold   &&   percentages.Lumber < percentages.Stone )
  {
    if( percentages.Gold < percentages.Stone )
      types = { ATree::StaticClass(), AGoldmine::StaticClass(), AStone::StaticClass() };
    else
      types = { ATree::StaticClass(), AStone::StaticClass(), AGoldmine::StaticClass() };
  }
  else // stone is scarecest
  {
    if( percentages.Lumber < percentages.Gold )
      types = { AStone::StaticClass(), ATree::StaticClass(), AGoldmine::StaticClass() };
    else
      types = { AStone::StaticClass(), AGoldmine::StaticClass(), ATree::StaticClass() }; // EQUAL
  }

  return types;
}

FString FAI::ToString()
{
  return FS( "AILevel aiLevel=%d foodFraction=%f scoutInterval=%f gold=%0.f lumber=%0.f stone=%0.f", 
    (int)aiLevel.GetValue(), foodFraction, scoutInterval,
    StartingResources.Gold, StartingResources.Lumber, StartingResources.Stone );
}


