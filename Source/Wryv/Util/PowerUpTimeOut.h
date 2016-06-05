#pragma once

#include "Game/UnitsData.h"

// Gives powerup to the player for fixed amount of time.
struct PowerUpTimeOut
{
  FUnitsData BonusStats;
  float timeRemaining; // How much time this bonus trait has left to be applied (per-frame)
  float nextApplicationIntervalIn; // How much time left before next application (refreshes when 0)

  PowerUpTimeOut() : timeRemaining(0.f), nextApplicationIntervalIn(0.f) { }
  PowerUpTimeOut( FUnitsData bonusStats ) :
    BonusStats( bonusStats ), timeRemaining( 0.f ), nextApplicationIntervalIn( 0.f )
  {
    timeRemaining = bonusStats.TimeLength;
    nextApplicationIntervalIn = bonusStats.Interval;
  }
  void Tick( float t ) {
    timeRemaining -= t;
    nextApplicationIntervalIn -= t; // t seconds have passed
  }
  bool IsReady() { return nextApplicationIntervalIn <= 0.f; }
  void Refresh() {
    nextApplicationIntervalIn = BonusStats.Interval;
  }
};


