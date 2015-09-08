#pragma once

#include "SlotPalette.h"

class AbilitiesPanel;
class BuildPanel;

class ActionsPanel : public HotSpot
{
public:
  // the group of abilities this unit is capable of
  AbilitiesPanel* abilitiesPanel;
  // the group of buildPanel this unit can build
  BuildPanel* buildPanel;

  ActionsPanel( FString name, FVector2D entrySize );
  void ShowAbilitiesPanel();
  void ShowBuildingsPanel();
  void Set( AGameObject* go );
};

class BuildPanel : public SlotPalette
{
  ActionsPanel *actions;
public:
  BuildPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( AGameObject *go );
};

// An abilities panel is a slotpalette but with the ability to populate
// from a game object's capabilities
class AbilitiesPanel : public SlotPalette
{
  ActionsPanel* actions;
public:
  // The Build button appears for Peasant-class units that build buildings.
  static UTexture* BuildButtonTexture;
  Clock* buildButton;

  // parent container is the actions panel
  AbilitiesPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( AGameObject *go );
};


