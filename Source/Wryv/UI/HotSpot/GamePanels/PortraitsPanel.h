#pragma once

#include "UI/HotSpot/Panels/FlowPanel.h"
#include "UI/HotSpot/Elements/Portrait.h"

class PortraitsPanel : public FlowPanel
{
public:
  static int MaxPortraits;
  PortraitsPanel( FVector2D size );
  Portrait* GetPortrait( int i );
  void Set( vector<AGameObject*> objects );
};


