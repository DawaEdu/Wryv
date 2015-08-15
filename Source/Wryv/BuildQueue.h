#pragma once

#include "StackPanel.h"
#include "GameObject.h"
#include "WryvGameInstance.h"

// This is the object that describes the units we are building
class BuildQueue : public StackPanel
{
public:
  FVector2D EntrySize;
  // A BuildQueue contains a list of things that are being built
  BuildQueue( FString name, FVector2D entrySize );
  void Set( AGameObject* go );

};
