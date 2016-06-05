#pragma once

#include "Cost.generated.h"

USTRUCT()
struct WRYV_API FCost
{
	GENERATED_USTRUCT_BODY()
public:
  FCost() {
    Gold = Lumber = Stone = 0.f;
  }
  FCost( float gold, float lumber, float stone ) {
    Gold = gold;
    Lumber = lumber;
    Stone = stone;
  }
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Cost )  float Gold;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Cost )  float Lumber;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Cost )  float Stone;

  FCost operator+( const FCost& cost ) {
    FCost c;
    c.Gold   = Gold + cost.Gold;
    c.Lumber = Lumber + cost.Lumber;
    c.Stone  = Stone + cost.Stone;
    return c;
  }

  FCost operator*( const FCost& cost ) {
    FCost c;
    c.Gold   = Gold * cost.Gold;
    c.Lumber = Lumber * cost.Lumber;
    c.Stone  = Stone * cost.Stone;
    return c;
  }

  FCost operator/( const FCost& cost ) {
    FCost c;
    c.Gold   = Gold / cost.Gold;
    c.Lumber = Lumber / cost.Lumber;
    c.Stone  = Stone / cost.Stone;
    return c;
  }

  FCost operator-() {
    FCost c;
    c.Gold   = -Gold;
    c.Lumber = -Lumber;
    c.Stone  = -Stone;
    return c;
  }
  
  FCost operator-( const FCost& cost ) {
    FCost c;
    c.Gold   = Gold - cost.Gold;
    c.Lumber = Lumber - cost.Lumber;
    c.Stone  = Stone - cost.Stone;
    return c;
  }

  FCost operator*( float mul ) {
    FCost c;
    c.Gold   = Gold * mul;
    c.Lumber = Lumber * mul;
    c.Stone  = Stone * mul;
    return c;
  }

  FCost& operator+=( const FCost& cost ) {
    Gold += cost.Gold;
    Lumber += cost.Lumber;
    Stone += cost.Stone;
    return *this;
  }

  FCost& operator-=( const FCost& cost ) {
    Gold -= cost.Gold;
    Lumber -= cost.Lumber;
    Stone -= cost.Stone;
    return *this;
  }

  FCost operator*=( float mul ) {
    Gold   *= mul;
    Lumber *= mul;
    Stone  *= mul;
    return *this;
  }

  bool operator>=( const FCost& cost ) {
    return Gold >= cost.Gold && Lumber >= cost.Lumber && Stone >= cost.Stone;
  }

};


