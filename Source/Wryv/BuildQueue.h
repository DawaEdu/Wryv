#pragma once

#include "GameObject.h"
#include "StackPanel.h"
#include "WryvGameInstance.h"

// This is the object that describes the units we are building
class BuildQueue : public StackPanel
{
public:
  AGameObject* Selected;
  FVector2D EntrySize;
  
  // A BuildQueue contains a list of things that are being built
  BuildQueue( FString name, FVector2D entrySize );
  void Set( AGameObject* go );
  virtual void render( FVector2D offset ) {
    StackPanel::render( offset ) ;
  }
};
