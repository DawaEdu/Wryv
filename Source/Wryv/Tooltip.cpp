#include "Wryv.h"
#include "Tooltip.h"

UTexture* Tooltip::TooltipBackgroundTexture = 0;

Tooltip::Tooltip():
  ITextWidget( "tooltip", TooltipBackgroundTexture, "tip", Alignment::CenterCenter )
{
}

