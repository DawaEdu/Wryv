#pragma once

#include "UI/HotSpot/Elements/Screen.h"

class TitleScreen : public Screen
{
public:
  ImageHS *graphic;
  TitleScreen( UTexture* titleTex, FVector2D size ) : Screen( "Title screen", size )
  {
    graphic = new ImageHS( "title graphic", titleTex );
    graphic->Align = Full;
    Add( graphic );
  }
};

