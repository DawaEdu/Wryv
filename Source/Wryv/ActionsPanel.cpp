#include "Wryv.h"
#include "ActionsPanel.h"
#include "Clock.h"
#include "WryvGameInstance.h"
#include "GameObject.h"

Actions::Actions( FString name, FVector2D entrySize ) : HotSpot( name )
{
  abilities = new AbilitiesPanel( this, SlotPalette::SlotPaletteTexture, 2, 3,
    entrySize, FVector2D(8,8) );
  Add( abilities );

  buildPanel = new BuildPanel( this, SlotPalette::SlotPaletteTexture, 2, 3,
    entrySize, FVector2D(8,8) );
  buildPanel->Hide();
  Add( buildPanel );

  recomputeSizeToContainChildren();
}

void Actions::ShowAbilitiesPanel()
{
  abilities->Show();
  buildPanel->Hide();
}

void Actions::ShowBuildingsPanel()
{
  buildPanel->Show();
  abilities->Hide();
}

void Actions::Set( AGameObject* go )
{
  abilities->Set( go );
  buildPanel->Set( go );
}



AbilitiesPanel::AbilitiesPanel( Actions* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  SlotPalette( "abilities panel", bkg, rows, cols, entrySize, pad )
{
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
  if( !go ) return;

  vector<Clock*> abilitiesClocks = Populate( go->Stats.Abilities );
  return;


  for( int i = 0; i < abilitiesClocks.size(); i++ )
  {
    Clock* clock = abilitiesClocks[ i ];
    // Attach button with invokation of i'th ability
    clock->OnMouseDownLeft = [go,i]( FVector2D mouse ) {
      // Invoke I'th action of the object
      go->UseAbility( i );
      return Consumed;
    };
  }
  
  buildButton->Hide();
  if( go->isPeasant() ) buildButton->Show();
  else if( go->isBuilding() ) {
    // show builds as 1st page items
    vector<Clock*> buildClocks = Populate( go->Stats.Builds );
    for( Clock* build : buildClocks )
    {
      // cancel the build if button clicked
      build->OnMouseDownLeft = [build](FVector2D mouse){
        info( FS( "Building a %s, %f complete",
          *GetTypesName( build->counter.Type ),
          build->counter.Percent() ) );
        return NotConsumed;
      };
    }
  }
  
}

BuildPanel::BuildPanel( Actions* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  SlotPalette( "BuildPanel", bkg, rows, cols, entrySize, pad )
{
  actions = iActions;
}

void BuildPanel::Set( AGameObject *go )
{
  HideChildren(); // Hide all buttons
  if( ! go )  return;

  

  // the rest of the abilities can be set to null
  for( int i = go->Stats.Builds.Num(); i < GetNumSlots(); i++ )
  {
    // Turn off the function object, jsut in case
    GetSlot(i)->OnMouseDownLeft = function<EventCode (FVector2D mouse)>(); // null the callback
    // hide slot 
    GetSlot(i)->Hide();
  }
  
}

