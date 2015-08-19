#pragma once

#include "Screen.h"

class TitleScreen : public Screen
{
public:
  ImageWidget *graphic;
  TitleScreen( UTexture* titleTex, FVector2D size ) : Screen( "Title screen", size )
  {
    graphic = new ImageWidget( "title graphic", titleTex );
    graphic->Align = Full;
    Add( graphic );
  }
};

