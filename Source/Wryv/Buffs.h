#pragma once

#include "StackPanel.h"

class Buffs : public StackPanel
{
public:
  AGameObject* selected;
  Buffs( FString name, UTexture* bkg ) : StackPanel( name, bkg ), selected( 0 )
  {
    Pad = FVector2D( 4, 4 );
  }
  void Set( AGameObject* go );
  virtual void Move( float t );
};
