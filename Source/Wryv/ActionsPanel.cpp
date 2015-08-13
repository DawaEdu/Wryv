#include "Wryv.h"
#include "ActionsPanel.h"
#include "Clock.h"
#include "WryvGameInstance.h"

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
  // Set abilities. Abilities panel always has [2 rows, 3 cols]
  // We line up each ability with the slot number.
  for( int i = 0; i < go->Abilities.size() && i < GetNumSlots()-1; i++ )
  {
    Ability ability = go->Abilities[i];
    FUnitsDataRow abilityData = Game->unitsData[ability.Type];
    
    // Want to change texture, it recenters the texture in case it is
    // the incorrect size for the slot.
    SetSlotTexture( i, abilityData.Portrait );
    Clock* clock = GetSlot( i );
    clock->Show();

    // Attach button with invokation of i'th ability
    clock->OnMouseDownLeft = [go,i,abilityData]( FVector2D mouse ) {
      LOG( "%s used ability %s", *go->Stats.Name, *abilityData.Name );
      go->UseAbility( go->Abilities[i] );
      return Consumed;
    };
  }

  // the rest of the abilities can be set to null
  for( int i = go->Abilities.size(); i < GetNumSlots(); i++ )
  {
    // Turn off the function object, jsut in case
    GetSlot(i)->OnMouseDownLeft = function<EventCode (FVector2D mouse)>(); // null the callback
    // hide slot 
    GetSlot(i)->Hide();
  }
  
  // Last slot will contain the build button.
  int lastSlot = GetNumSlots()-1;
  if( lastSlot >= 0 )
  {
    Clock* buildSlot = GetSlot( lastSlot );
    buildSlot->Show();
  }

  if( go->Stats.Type == Types::UNITPEASANT )
  {
    buildButton->Show();
  }
}

BuildPanel::BuildPanel( Actions* iActions, UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) : 
  SlotPalette( "BuildPanel", bkg, rows, cols, entrySize, pad )
{
  actions = iActions;
}

void BuildPanel::Set( AGameObject *go )
{
  for( int i = 0; i < go->Stats.Spawns.Num(); i++ )
  {
    Types spawn = go->Stats.Spawns[i];

    // Construct buttons that run abilities of the object.
    SetSlotTexture( i, Game->GetPortrait( spawn ) );
    ITextWidget* button = GetSlot( i );
    button->OnMouseDownLeft = [go,i]( FVector2D mouse ){
      // try spawn the object type listed
      go->Build( go->Stats.Spawns[i] );
      return Consumed;
    };
  }
}

