#pragma once

#include "FlowPanel.h"
#include "Portrait.h"

class PortraitsPanel : public FlowPanel
{
public:
  static int MaxPortraits;
  PortraitsPanel( FVector2D size );
  Portrait* GetPortrait( int i );
  void Set( vector<AGameObject*> objects );
};


