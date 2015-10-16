#include "Wryv.h"

#include "Action.h"
#include "ActionsPanel.h"
#include "Building.h"
#include "Clock.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "Peasant.h"
#include "Unit.h"
#include "WryvGameInstance.h"

#include "Research.h"
#include "TrainingAction.h"
#include "UnitAction.h"

UTexture* AbilitiesPanel::BuildButtonTexture = 0;

ActionsPanel::ActionsPanel( FString name, FVector2D entrySize ) : HotSpot( name )
{
  Align = TopCenter;

  abilitiesPanel = new AbilitiesPanel( this, SlotPalette::SlotPaletteTexture, 2, 3,
    entrySize, FVector2D(8,8) );
  Add( abilitiesPanel );

  buildPanel = new BuildPanel( this, SlotPalette::SlotPaletteTexture, 2, 3,
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

void ActionsPanel::Set( AGameObject* go )
{
  abilitiesPanel->Set( go );
  buildPanel->Set( go );
}



AbilitiesPanel::AbilitiesPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  SlotPalette( "abilities panel", bkg, rows, cols, entrySize, pad )
{
  Align = TopCenter;
  Actions = iActions;
  if( GetNumActiveSlots() )
  {
    buildButton = GetClock( GetNumActiveSlots() - 1 );
    buildButton->Tex = BuildButtonTexture;
    buildButton->OnMouseDownLeft = [this]( FVector2D mouse ) -> EventCode
    {
      Actions->ShowAbilitiesPanel();
      return Consumed;
    };
  }
}

void AbilitiesPanel::Set( AGameObject *go )
{
  // start by hiding all abilities
  HideChildren();
  
  if( AUnit* unit = Cast< AUnit >( go ) )
  {
    Populate<UUnitAction>( unit->CountersAbility, 0 );
  }
  else if( ABuilding* building = Cast< ABuilding >( go ) )
  {
    Populate<UTrainingAction>( building->TrainingAvailable, 0 );
    // Put the researches in also
    Populate<UResearch>( building->ResearchesAvailable, building->TrainingAvailable.Num() );
  }
}



BuildPanel::BuildPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  SlotPalette( "BuildPanel", bkg, rows, cols, entrySize, pad )
{
  Align = TopCenter;
  Actions = iActions;
}

void BuildPanel::Set( AGameObject *go )
{
  HideChildren(); // Hide all buttons
  
  // TODO: Populate with peasant's buildables.
  // Enable the build button etc.
  if( APeasant* peasant = Cast<APeasant>( go ) )
  {
    // the rest of the abilities can be set to null
    for( int i = 0; i < GetNumActiveSlots(); i++ )
    {
      // Turn off the function object, jsut in case
      GetClock(i)->OnMouseDownLeft = function<EventCode (FVector2D mouse)>(); // null the callback
      // hide slot 
      GetClock(i)->Hide();
    }
  }
  else if( ABuilding* building = Cast<ABuilding>( go ) )
  {
    
  }
}

