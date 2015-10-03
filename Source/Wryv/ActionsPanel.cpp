#include "Wryv.h"

#include "Action.h"
#include "ActionsPanel.h"
#include "Building.h"
#include "Clock.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "Peasant.h"
#include "Unit.h"
#include "UnitAction.h"
#include "WryvGameInstance.h"

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
  actions = iActions;
  if( GetNumSlots() )
  {
    buildButton = GetSlot( GetNumSlots() - 1 );
    buildButton->Tex = BuildButtonTexture;
    buildButton->OnMouseDownLeft = [this]( FVector2D mouse ) {
      actions->ShowAbilitiesPanel();
      return Consumed;
    };
  }
}

void AbilitiesPanel::Set( AGameObject *go )
{
  // start by hiding all abilities
  HideChildren();
  AUnit* unit = Cast< AUnit >( go );
  if( !unit ) return;

  unit->InitIcons();

  vector<Clock*> abilitiesClocks = Populate<UUnitAction>( unit->CountersAbility );

  for( int i = 0; i < abilitiesClocks.size(); i++ )
  {
    Clock* clock = abilitiesClocks[ i ];

    // Attach button with invokation of i'th ability
    clock->OnMouseDownLeft = [unit,i]( FVector2D mouse ) {
      // Invoke I'th action of the object
      unit->CountersAbility[i]->Click();
      return Consumed;
    };
  }
}



BuildPanel::BuildPanel( ActionsPanel* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  SlotPalette( "BuildPanel", bkg, rows, cols, entrySize, pad )
{
  Align = TopCenter;
  actions = iActions;
}

void BuildPanel::Set( AGameObject *go )
{
  HideChildren(); // Hide all buttons
  

  if( APeasant* peasant = Cast<APeasant>( go ) )
  {
    
    // the rest of the abilities can be set to null
    int i = 0;
    for( ; i < GetNumSlots(); i++ )
    {
      // Turn off the function object, jsut in case
      GetSlot(i)->OnMouseDownLeft = function<EventCode (FVector2D mouse)>(); // null the callback
      // hide slot 
      GetSlot(i)->Hide();
    }

    for( ; i < GetNumSlots(); i++ )
    {
      // Turn off the function object, jsut in case
      GetSlot(i)->OnMouseDownLeft = function<EventCode (FVector2D mouse)>(); // null the callback
      // hide slot 
      GetSlot(i)->Hide();
    }


  }
  else if( ABuilding* building = Cast<ABuilding>( go ) )
  {
    
  }

  
}

