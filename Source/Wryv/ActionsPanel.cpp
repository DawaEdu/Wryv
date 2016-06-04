#include "Wryv.h"

#include "UIActionCommand.h"
#include "ActionsPanel.h"
#include "Building.h"
#include "Clock.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Peasant.h"
#include "Unit.h"
#include "WryvGameInstance.h"

#include "UIResearchCommand.h"
#include "UITrainingActionCommand.h"
#include "UIUnitActionCommand.h"

UTexture* AbilitiesPanel::BuildButtonTexture = 0;

ActionsPanel::ActionsPanel( FString name, FVector2D entrySize ) : HotSpot( name )
{
  Align = TopCenter;

  abilitiesPanel = new AbilitiesPanel( this, SlotPanel::SlotPanelTexture, 2, 3,
    entrySize, FVector2D(8,8) );
  Add( abilitiesPanel );

  buildPanel = new BuildPanel( this, SlotPanel::SlotPanelTexture, 2, 3,
    entrySize, FVector2D(8,8) );
  buildPanel->Hide();
  Add( buildPanel );

  recomputeSizeToContainChildren();
}

void ActionsPanel::ShowAbilitiesPanel()
{
  abilitiesPanel->Show();
  buildPanel->Hide();
}

void ActionsPanel::ShowBuildingsPanel()
{
  buildPanel->Show();
  abilitiesPanel->Hide();
}

void ActionsPanel::Set( vector<AGameObject*> objects )
{
  abilitiesPanel->Set( objects );
  buildPanel->Set( objects );
}



AbilitiesPanel::AbilitiesPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, 
  FVector2D entrySize, FVector2D pad ) : 
  SlotPanel( "abilities panel", bkg, rows, cols, entrySize, pad )
{
  Align = TopCenter;
  Actions = iActions;
  if( GetNumActiveSlots() )
  {
    buildButton = (Clock*)GetChild( GetNumActiveSlots() - 1 );
    buildButton->Tex = BuildButtonTexture;
    buildButton->OnMouseDownLeft = [this]( FVector2D mouse ) -> EventCode
    {
      Actions->ShowAbilitiesPanel();
      return Consumed;
    };
  }
}

void AbilitiesPanel::Set( vector<AGameObject*> objects )
{
  // start by hiding all abilities
  Blank();
  if( !objects.size() )  return;
  
  // the abilities we use have to be properties of ALL objects selected for them to be used.
  // use the front object to populate abilities.
  AGameObject* go = *objects.begin();
  
  if( AUnit* unit = Cast<AUnit>( go ) )
  {
    Populate<UUIUnitActionCommand>( unit->CountersAbility, 0 );
  }
  else if( ABuilding* building = Cast< ABuilding >( go ) )
  {
    if( building->TrainingAvailable.Num() )
      Populate<UUITrainingActionCommand>( building->TrainingAvailable, 0 );
    if( building->ResearchesAvailable.Num() )
      Populate<UUIResearchCommand>( building->ResearchesAvailable, building->TrainingAvailable.Num() );
  }
}



BuildPanel::BuildPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  SlotPanel( "BuildPanel", bkg, rows, cols, entrySize, pad )
{
  Align = TopCenter;
  Actions = iActions;
}

void BuildPanel::Set( vector<AGameObject*> objects )
{
  HideChildren(); // Hide all buttons
  if( !objects.size() )  return;

  AGameObject* go = objects.front();

  // TODO: Populate with peasant's buildables.
  // Enable the build button etc.
  if( APeasant* peasant = Cast<APeasant>( go ) )
  {
    // the rest of the abilities can be set to null
    for( int i = 0; i < GetNumActiveSlots(); i++ )
    {
      // Turn off the function object, just in case
      GetChild(i)->OnMouseDownLeft = function<EventCode (FVector2D mouse)>(); // null the callback
      // hide slot 
      GetChild(i)->Hide();
    }
  }
  else if( ABuilding* building = Cast<ABuilding>( go ) )
  {
    
  }
}

