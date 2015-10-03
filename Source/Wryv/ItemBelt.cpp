#include "Wryv.h"

#include "Building.h"
#include "Item.h"
#include "ItemAction.h"
#include "ItemBelt.h"
#include "ITextWidget.h"
#include "TheHUD.h"
#include "Unit.h"
#include "WryvGameInstance.h"

ItemBelt::ItemBelt( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) :
  SlotPalette( "itembelt", bkg, rows, cols, entrySize, pad )
{
  Align = BottomCenter;
}

void ItemBelt::Set( AGameObject* go )
{
  // No items in the belt. Empty the belt.
  if( AUnit* unit = Cast<AUnit>( go ) )
  {
    // repopulate the # grid slots according to # items unit has
    if( unit->CountersItems.size() )
    {
      // Populate the toolbelt, etc
      int itemRows = 1 + ( (unit->CountersItems.size() - 1) / 4 ); // 1 + ( 5 - 1 )/4
      int itemCols = 4;
      itemCols = unit->CountersItems.size() % 4;
      if( !itemCols )  itemCols = 4;

      vector<Clock*> clocks = SetNumSlots( itemRows, itemCols );
    
      // The function associated with the Item is hooked up here.
      // Inventory size dictates #items.
      for( int i = 0; i < clocks.size(); i++ )
      {
        Clock* slot = clocks[i];

        UItemAction* itemAction = unit->CountersItems[i];
        SetSlotTexture( i, itemAction->Icon );

        // Trigger the gameobject to consume i'th item.
        GetSlot( i )->OnMouseDownLeft = [this,i,unit](FVector2D mouse) -> EventCode {
          unit->UseItem( i );
          return Consumed;
        };

        GetSlot(i)->OnHover = [itemAction,slot](FVector2D mouse) -> EventCode
        {
          // display a tooltip describing the current item.
          // or could add as a child of the img widget
          ITextWidget* tooltip = Game->hud->ui->gameChrome->tooltip;
          tooltip->Set( itemAction->Text );
          tooltip->Align = HCenter | OnTopOfParent;
          // put the tooltip as a child of the slot
          slot->Add( tooltip );
          return Consumed;
        };
      }
    }
  }
  else
  {
    // Clear the old items.
    SetNumSlots(0, 0);
  }
}

