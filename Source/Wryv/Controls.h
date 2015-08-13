#pragma once

#include "StackPanel.h"

class Controls : public StackPanel
{
public:
  ImageWidget *Pause;

  Controls( UTexture* texPause ) : StackPanel( "Controls" )
  {
    Pause = new ImageWidget( "pause", texPause );
    StackRight( Pause );
  }
};

