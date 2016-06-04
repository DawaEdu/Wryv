#pragma once

#include "SlotPanel.h"

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
  void Set( vector<AGameObject*> objects );
};

class BuildPanel : public SlotPanel
{
  ActionsPanel *Actions;
public:
  BuildPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( vector<AGameObject*> objects );
};

// An abilities panel is a SlotPanel but with the ability to populate
// from a game object's capabilities
class AbilitiesPanel : public SlotPanel
{
  ActionsPanel* Actions;
public:
  // The Build button appears for Peasant-class units that build buildings.
  static UTexture* BuildButtonTexture;
  Clock* buildButton;

  // parent container is the Actions panel
  AbilitiesPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad );
  void Set( vector<AGameObject*> objects );
};


