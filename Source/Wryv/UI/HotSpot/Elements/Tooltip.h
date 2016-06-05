#pragma once

#include "UI/HotSpot/Elements/IText.h"

class Tooltip : public IText
{
public:
  static UTexture* TooltipBackgroundTexture;
  Tooltip();
};