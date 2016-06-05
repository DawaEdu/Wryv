#pragma once

#include "UI/HotSpot/Panels/StackPanel.h"
#include "UI/HotSpot/Elements/ProgressBar.h"

class TextWidget;
class BuffsPanel;

class StatsPanel : public StackPanel
{
  TextWidget* unitName;
  ProgressBar* hpBar;
  BuffsPanel* buffs;        // list of buffs applied to this unit
  TextWidget* hpText;
  TextWidget* damage;
  TextWidget* armor;
  TextWidget* description;
  ResourcesPanel* resourcesCarrying;
  float BarSize;
public:
  StatsPanel();
  void Blank();
  void Restack();
  void Set( vector<AGameObject*> objects );
  virtual void render( FVector2D offset );
};
