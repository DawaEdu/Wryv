#pragma once

#include "StackPanel.h"
#include "ProgressBar.h"

class TextWidget;

class StatsPanel : public StackPanel
{
  TextWidget* unitName;
  TextWidget* hp;
  TextWidget* damage;
  TextWidget* armor;
  TextWidget* description;
  ProgressBar* progress;
  AGameObject* Selected;

public:
  StatsPanel( FString iname, UTexture* tex, FLinearColor color );
  void Blank();
  void Restack();
  void Set( AGameObject* go );
  virtual void Move( float t );
};
