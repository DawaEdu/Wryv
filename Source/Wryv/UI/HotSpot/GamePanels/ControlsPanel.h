#pragma once

#include "UI/HotSpot/Panels/StackPanel.h"

class ControlsPanel : public StackPanel
{
public:
  static UTexture* PauseButtonTexture;
  static UTexture* ResumeButtonTexture;
  ImageHS *Pause;

  ControlsPanel();
};

