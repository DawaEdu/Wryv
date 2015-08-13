#pragma once

#include "Screen.h"

class TitleScreen : public Screen
{
public:
  ImageWidget *graphic;
  TitleScreen( UTexture* titleTex ) : Screen( "Title screen" )
  {
    graphic = new ImageWidget( "title graphic", titleTex );
    graphic->Align = Full;
    Add( graphic );
  }
};

