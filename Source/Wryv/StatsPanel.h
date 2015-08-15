#pragma once

#include "StackPanel.h"

class TextWidget;

class StatsPanel : public StackPanel
{
  TextWidget* unitName;
  TextWidget* hp;
  TextWidget* damage;
  TextWidget* armor;
  TextWidget* description;
public:
  StatsPanel( FString iname, UTexture* tex, FLinearColor color );
  void Blank();
  void Set( AGameObject* go );

};
