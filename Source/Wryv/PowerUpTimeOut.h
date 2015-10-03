#pragma once

class AItem;

struct PowerUpTimeOut
{
  AItem* Powerup;
  float timeRemaining;
  float timeInterval;  //application interval between Use() calls
  PowerUpTimeOut():Powerup(0),timeRemaining(0.f),timeInterval(0.f){}
  PowerUpTimeOut( AItem* item, float timeLength, float interval ) :
    Powerup( item ), timeRemaining( timeLength ), timeInterval( interval ) { }
};


