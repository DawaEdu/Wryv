#pragma once

#include "StackPanel.h"
#include "GameObject.h"
#include "WryvGameInstance.h"

// This is the object that describes the units we are building
class BuildQueue : public StackPanel
{
public:
  AGameObject* Selected;
  FVector2D EntrySize;
  static FLinearColor ClockColor;
  bool needsRefresh;
  vector<Clock*> clocks;

  // A BuildQueue contains a list of things that are being built
  BuildQueue( FString name, FVector2D entrySize );
  virtual void Clear();
  void Set( AGameObject* go );
  void Refresh();
  virtual void Move( float t );
  virtual void render( FVector2D offset ) {
    StackPanel::render( offset ) ;
  }
};
