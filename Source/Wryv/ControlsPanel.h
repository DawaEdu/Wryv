#pragma once

#include "StackPanel.h"

class ControlsPanel : public StackPanel
{
public:
  static UTexture* PauseButtonTexture;
  static UTexture* ResumeButtonTexture;
  ImageHS *Pause;

  ControlsPanel();
};

