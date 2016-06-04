#include "Wryv.h"
#include "Tooltip.h"

UTexture* Tooltip::TooltipBackgroundTexture = 0;

Tooltip::Tooltip():
  IText( "tooltip", TooltipBackgroundTexture, "tip", Alignment::CenterCenter )
{
}

