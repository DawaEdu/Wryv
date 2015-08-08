#include "Wryv.h"
#include "Unit.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "PlayerControl.h"
#include "TheHUD.h"
#include "Widget.h"
#include "UnitsData.h"

#include <map>
using namespace std;

// Sets default values
AUnit::AUnit( const FObjectInitializer& PCIP ) : AGameObject( PCIP )
{
 	// Set this actor to call Tick() every frame.
  // You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
  attackTarget = 0;
}

void AUnit::BeginPlay()
{
  Super::BeginPlay();
  for( int i = 0; i < StartingItems.Num(); i++ )
  {
    Items.Push( Game->unitsData[ StartingItems[i] ] );
  }
}

void AUnit::OnSelected()
{
  AGameObject::OnSelected();
  SlotPalette* itemBelt = Game->hud->ui->gameChrome->itemBelt;

  if( Items.Num() )
  {
    // Move selection cursor to this unit.
    // Populate the toolbelt, etc
    int itemRows = 1 + ((Items.Num() - 1) / 4); // 1 + ( 5 - 1 )/4
    int itemCols = 4;
    itemCols = Items.Num() % 4;
    if( !itemCols )  itemCols = 4;

    vector<ITextWidget*> slots = itemBelt->SetNumSlots( itemRows, itemCols );
    
    // The function associated with the Item is hooked up here.
    // Inventory size dictates #items.
    for( int i = 0; i < slots.size(); i++ )
    {
      ITextWidget* slot = slots[i];
      FUnitsDataRow item = Items[i];
      itemBelt->SetSlotTexture( i, item.Icon );
      itemBelt->GetSlot( i )->OnMouseDownLeft = [this,i](FVector2D mouse){
        // use the item. qty goes down by 1
        Items[i].Quantity--; // we don't affect the UI here, only
        // the `model` object of the player. Items[i] gets reflushed each frame.
        ApplyEffect( Items[i] );
        if( !Items[i].Quantity )
          Items[i].Type = Types::NOTHING;
        return Consumed;
      };

      Tooltip* tooltip = Game->hud->ui->gameChrome->tooltip;
      itemBelt->GetSlot(i)->OnHover = [slot,item,tooltip](FVector2D mouse)
      {
        // display a tooltip describing the current item.
        // or could add as a child of the img widget
        tooltip->Set( item.Name + FString(": ") + item.Description, 5.f );
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
    itemBelt->SetNumSlots(0, 0);
  }
}

void AUnit::Move( float t )
{
  // Tick all the traits
  for( int i = BonusTraits.size() - 1; i >= 0; i-- ) {
    BonusTraits[i].time -= t;
    if( BonusTraits[i].time <= 0 )
      BonusTraits.erase( BonusTraits.begin() + i );
  }
}


