#pragma once

#include "CooldownCounter.generated.h"

// Structure representing something in progress.
USTRUCT()
struct WRYV_API FCooldownCounter
{
  GENERATED_USTRUCT_BODY()
  float Time;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)
  float TotalTime;
  FCooldownCounter()
  {
    Time = 0.f;
    TotalTime = 0.f;
  }
  float Fraction()
  {
    // If TotalTime is set, use it to determine fraction.
    // Else, the cooldown has 0 recharge time, so it is always fully charged.
    if( TotalTime )
      return Time / TotalTime;
    else
      return 1.f; // always recharged.
  }
  bool Done() { return Time >= TotalTime; }
  void Set( float time ) { Time = time;
    if( Time > TotalTime )
      Time = TotalTime; }
  void Reset() { Time = 0.f; }
  void Step( float t ) {
    Time += t;

    // Prevent over 100% counts
    if( Time > TotalTime )
      Time = TotalTime;
  }
};




