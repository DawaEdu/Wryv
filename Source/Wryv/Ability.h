#pragma once

#include "Types.h"

// Structure representing a building (or unit) that is in progress
struct CooldownCounter
{
  Types Type; // The type of unit that is building or that can cast.
  float Time, TotalTime; // Time it has been building for, and total time it will build for
  
  CooldownCounter() : Time(0.f), TotalTime(1.f), Type(Types::NOTHING) {}
  CooldownCounter( Types type );
  float Percent() { return Time/TotalTime; }
  void Tick( float t ){ Time += t ; }
  bool Done() { return Time >= TotalTime; }
  void Reset() { Time = 0.f; }
};

struct Ability : public CooldownCounter
{
  bool Enabled;

  Ability( Types type );
};

