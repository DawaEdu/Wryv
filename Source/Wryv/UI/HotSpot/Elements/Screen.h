#pragma once

#include "UI/HotSpot/Elements/HotSpot.h"

class Screen : public HotSpot
{
public:
  Screen( FString name, FVector2D size ) : HotSpot( name, size ){
    Align = Full;
  }

  // Screens are usually full screen, but have a lot of translucent areas.
  bool hitsElement( FVector2D mouse )
  {
    for( int i = 0; i < GetNumChildren(); i++ )
      if( GetChild(i)->hit( mouse ) )
        return 1;
    return 0;
  }
};

