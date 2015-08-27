#pragma once

#include "ITextWidget.h"

class Tooltip : public ITextWidget
{
public:
  static UTexture* TooltipBackgroundTexture;
  Tooltip();
};