#pragma once

#include "StackPanel.h"

class StatsPanel : public StackPanel
{
  TextWidget* unitName;
  TextWidget* hp;
  TextWidget* damage;
  TextWidget* armor;
  TextWidget* description;
public:
  StatsPanel( FString iname, UTexture* tex, FLinearColor color ) :
    StackPanel( iname, tex, color )
  { 
    unitName = new TextWidget( "" );
    StackBottom( unitName );
    hp = new TextWidget( "" );
    StackBottom( hp );
    damage = new TextWidget( "" );
    StackBottom( damage );
    armor = new TextWidget( "" );
    StackBottom( armor );
    description = new TextWidget( "" );
    StackBottom( description );
  }

  void Set( AGameObject* go )
  {
    // set the text inside with gameobject
    //LOG( "Set text with gameobject" );
    //this->
  }
};
