#pragma once

#include "IText.h"

class Tooltip : public IText
{
public:
  static UTexture* TooltipBackgroundTexture;
  Tooltip();
};