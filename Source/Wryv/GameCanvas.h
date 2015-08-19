#pragma once

#include "Border.h"
#include "Screen.h"
#include "ImageWidget.h"
#include "CursorTexture.h"

class GameCanvas : public Screen
{
public:
  ImageWidget* cursor;
  MouseSelectBox* selectBox;
  static FCursorTexture MouseCursorHand;
  static FCursorTexture MouseCursorCrossHairs;

  GameCanvas( FVector2D size );
  void Set( FVector2D mouse );
  void SelectStart( FVector2D mouse );
  void DragBox( FVector2D mouse );
  void SelectEnd();
};


