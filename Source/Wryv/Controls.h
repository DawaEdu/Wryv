#pragma once

#include "StackPanel.h"

class Controls : public StackPanel
{
public:
  static UTexture* PauseButtonTexture;
  static UTexture* ResumeButtonTexture;
  ImageWidget *Pause;

  Controls();
};

