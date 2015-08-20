#pragma once

#include "StackPanel.h"
#include "ProgressBar.h"

class TextWidget;

class StatsPanel : public StackPanel
{
  TextWidget* unitName;
  ProgressBar* hpBar;
  TextWidget* hpText;
  TextWidget* damage;
  TextWidget* armor;
  TextWidget* description;
  AGameObject* Selected;
public:
  StatsPanel();
  void Blank();
  void Restack();
  void Set( AGameObject* go );
  virtual void Move( float t );
};
