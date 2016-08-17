#pragma once

#include "UI/HotSpot/Elements/Border.h"
#include "UI/HotSpot/Elements/Screen.h"
#include "UI/HotSpot/Elements/Image.h"
#include "Util/CursorTexture.h"

class GameCanvas : public Screen
{
public:
  Image* cursor;
  MouseSelectBox* selectBox;
  static FCursorTexture MouseCursorHand;
  static FCursorTexture MouseCursorCrossHairs;

  GameCanvas( FVector2D size );
  void Set( FVector2D mouse );
  void BoxSelectStart( FVector2D mouse );
  void BoxDrag( FVector2D mouse );
  void BoxSelectEnd();
};


