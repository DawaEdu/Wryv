#include "Wryv.h"
#include "Widget.h"
#include "GameFramework/HUD.h"
#include "TheHUD.h"
#include "GameObject.h"
#include "Unit.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

TextWidget* HotSpot::TooltipWidget = 0;
UTexture* ImageWidget::NullTexture = 0;

UTexture* SlotPalette::SlotPaletteTexture = 0;
UTexture* StackPanel::StackPanelTexture = 0;
UTexture* AbilitiesPanel::BuildButtonTexture = 0;
UTexture* ResourcesWidget::GoldTexture=0;
UTexture* ResourcesWidget::LumberTexture=0;
UTexture* ResourcesWidget::StoneTexture=0;

ItemBelt::ItemBelt( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) :
  SlotPalette( "itembelt", bkg, rows, cols, entrySize, pad )
{
  
}

void ItemBelt::Set( AUnit *unit )
{
  if( !unit )  return;

  // repopulate the # grid slots according to # items unit has
  else if( unit->Items.Num() )
  {
    // Populate the toolbelt, etc
    int itemRows = 1 + ((unit->Items.Num() - 1) / 4); // 1 + ( 5 - 1 )/4
    int itemCols = 4;
    itemCols = unit->Items.Num() % 4;
    if( !itemCols )  itemCols = 4;

    vector<Clock*> clocks = SetNumSlots( itemRows, itemCols );
    
    // The function associated with the Item is hooked up here.
    // Inventory size dictates #items.
    for( int i = 0; i < clocks.size(); i++ )
    {
      Clock* slot = clocks[i];
      FUnitsDataRow item = unit->Items[i];
      SetSlotTexture( i, item.Portrait );

      // Trigger the gameobject to consume i'th item.
      GetSlot( i )->OnMouseDownLeft = [this,i,unit](FVector2D mouse){
        unit->ConsumeItem( i );
        return Consumed;
      };

      ITextWidget* tooltip = Game->hud->ui->gameChrome->tooltip;
      GetSlot(i)->OnHover = [slot,item,tooltip](FVector2D mouse)
      {
        // display a tooltip describing the current item.
        // or could add as a child of the img widget
        tooltip->Set( item.Name + FString(": ") + item.Description );
        tooltip->Align = HCenter | OnTopOfParent;
        // put the tooltip as a child of the slot
        slot->Add( tooltip );
        return Consumed;
      };
    }
  }
  else
  {
    // No items in the belt. Empty the belt.
    SetNumSlots(0, 0);
  }
}

void Buffs::Set( AGameObject* go )
{
  selected = go; // save for tick
  Clear();
  if( !go ) return;
  
  for( int i = 0; i < go->BonusTraits.size(); i++ )
  {
    Types buff = go->BonusTraits[i].traits.Type;
    StackRight( new ImageWidget( "a buff", Game->GetPortrait( buff ) ) );
  }
}

void Buffs::Move( float t )
{
  Set( selected );
}

void Actions::Set( AGameObject* go )
{
  buildPanel->Set( go );
}

void GameChrome::Select( vector<AGameObject*> objects )
{
  if( !objects.size() )
  {
    // clear selection.
    LOG( "GameChrome: Selection empty" );
    return;
  }

  rightPanel->Set( objects );
  


  // on selection, populate the itemBelt and other widgets
  // Clear the items palette
  itemBelt->Set( Cast<AUnit>(objects[0]) );
  
  // Building types have a build queue
  buildQueue->Set( objects[0] );

  // buffs. PER-FRAME: Clear & then re-draw the buffs
  buffs->Set( objects[0] );
}
