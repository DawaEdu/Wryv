#pragma once

#include "StackPanel.h"
#include "ProgressBar.h"

class TextWidget;
class Buffs;

class StatsPanel : public StackPanel
{
  TextWidget* unitName;
  ProgressBar* hpBar;
  Buffs*      buffs;        // list of buffs applied to this unit
  TextWidget* hpText;
  TextWidget* damage;
  TextWidget* armor;
  TextWidget* description;
  ResourcesWidget* resourcesCarrying;
  float BarSize;
public:
  StatsPanel();
  void Blank();
  void Restack();
  void Set( vector<AGameObject*> objects );
  virtual void render( FVector2D offset );
};
