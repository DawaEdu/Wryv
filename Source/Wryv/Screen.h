#pragma once

#include "HotSpot.h"

class Screen : public HotSpot
{
public:
  Screen( FString name ) : HotSpot( name ){
    Align = Full;
  }
};

