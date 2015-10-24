#include "Wryv.h"
#include "HotSpot.h"
#include "TextWidget.h"

void HotSpot::Reset()
{
  OnMouseDownLeft = function< EventCode (FVector2D mouse) >();
  OnMouseUpLeft = function< EventCode (FVector2D mouse) >();
  OnMouseDownRight = function< EventCode (FVector2D mouse) >();
  OnMouseUpRight = function< EventCode (FVector2D mouse) >();
  OnMouseDragLeft = function< EventCode (FVector2D mouse) >();

  OnHover = [this](FVector2D mouse) -> EventCode {
    if( !TooltipText.IsEmpty() )
      TooltipWidget->Set( TooltipText );
    return NotConsumed;
  };

  XLimits = 0, YLimits = 0;
}