#pragma once

#include "UI/HotSpot/Elements/Screen.h"

class TitleScreen : public Screen
{
public:
  Image *graphic;
  TitleScreen( UTexture* titleTex, FVector2D size ) : Screen( "Title screen", size )
  {
    graphic = new Image( "title graphic", titleTex );
    graphic->Align = Full;
    Add( graphic );
  }
};

