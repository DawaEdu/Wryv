#include "Wryv.h"
#include "ItemBelt.h"

ItemBelt::ItemBelt( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) :
  SlotPalette( "itembelt", bkg, rows, cols, entrySize, pad )
{
  Align = BottomCenter;
}

void ItemBelt::Set( AUnit *unit )
{
  // No items in the belt. Empty the belt.
  if( !unit )
  {
    // Clear the old items.
    SetNumSlots(0, 0);
    return;
  }

  // repopulate the # grid slots according to # items unit has
  if( unit->Items.Num() )
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
      GetSlot( i )->OnMouseDownLeft = [this,i,unit](FVector2D mouse) -> EventCode {
        unit->ConsumeItem( i );
        return Consumed;
      };

      ITextWidget* tooltip = Game->hud->ui->gameChrome->tooltip;
      GetSlot(i)->OnHover = [slot,item,tooltip](FVector2D mouse) -> EventCode
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
}

