#pragma once

#include "Border.h"
#include "Screen.h"
#include "ImageHS.h"
#include "CursorTexture.h"

class GameCanvas : public Screen
{
public:
  ImageHS* cursor;
  MouseSelectBox* selectBox;
  static FCursorTexture MouseCursorHand;
  static FCursorTexture MouseCursorCrossHairs;

  GameCanvas( FVector2D size );
  void Set( FVector2D mouse );
  void SelectStart( FVector2D mouse );
  void DragBox( FVector2D mouse );
  void SelectEnd();
};


