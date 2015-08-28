#pragma once

#include "FlowPanel.h"

class PortraitsPanel : public FlowPanel
{
public:
  PortraitsPanel( FVector2D size );
  void Set( set<AGameObject*> objects );
  
};


