#include "Wryv.h"
#include "UI/HotSpot/Elements/Tooltip.h"

UTexture* Tooltip::TooltipBackgroundTexture = 0;

Tooltip::Tooltip():
  IText( "tooltip", TooltipBackgroundTexture, "tip", Alignment::CenterCenter )
{
}

