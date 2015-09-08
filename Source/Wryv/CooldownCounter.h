#pragma once

#include "Types.h"

class AGameObject;

// Structure representing something in progress.
// Cooldown for building something, recharging something etc.
struct CooldownCounter
{
  Types Type;
  float Time, TotalTime;
  AGameObject* go; // The object that is being constructed

  CooldownCounter();
  CooldownCounter( Types type );
  CooldownCounter( Types type, AGameObject* object );
  float Fraction();
  bool Done() { return Time >= TotalTime; }
  void Reset() { Time = 0.f; }
};




