#include "RTSGame.h"
#include "Unit.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "PlayerControl.h"
#include "MyHUD.h"
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

void AUnit::ApplyEffect( FUnitsDataRow item )
{
  // timeLength, dataSet
  UE_LOG( LogTemp, Warning, TEXT("Applying %s for %f seconds"),
    *item.Name, item.TimeLength );

  // don't do anything for the Nothing item
  if( !IsItem( item.Type ) )
  {
    UE_LOG( LogTemp, Warning, TEXT("%s NOT AN ITEM"), *item.Name );
  }
  else
  {
    BonusTraits.push_back( PowerUpTimeOut( item.TimeLength, item ) );
  }
}

void AUnit::OnSelected()
{
  AGameObject::OnSelected();
  SlotPalette* itemBelt = Game->myhud->ui->itemBelt;

  if( Items.Num() )
  {
    // Move selection cursor to this unit.
    // Populate the toolbelt, etc
    int itemRows = 1 + ((Items.Num() - 1) / 4); // 1 + ( 5 - 1 )/4
    int itemCols = 4;
    itemCols = Items.Num() % 4;
    if( !itemCols )  itemCols = 4;

    vector<SlotEntry*> slots = itemBelt->SetNumSlots( itemRows, itemCols );
    
    // The function associated with the Item is hooked up here.
    // Inventory size dictates #items.
    for( int i = 0; i < slots.size(); i++ )
    {
      SlotEntry* slot = slots[i];
      FUnitsDataRow item = Items[i];
      FWidgetData &data = Game->myhud->widgets[ item.Type ];
      itemBelt->SetSlotTexture( i, data.Icon );
      itemBelt->GetSlot( i )->OnClicked = [this,i](FVector2D mouse){
        // use the item. qty goes down by 1
        Items[i].Quantity--; // we don't affect the UI here, only
        // the `model` object of the player. Items[i] gets reflushed each frame.
        ApplyEffect( Items[i] );
        if( !Items[i].Quantity )
          Items[i].Type = Types::NOTHING;
        return 0;
      };

      Tooltip* tooltip = Game->myhud->ui->tooltip;
      itemBelt->GetSlot(i)->OnHover = [slot,item,data,tooltip](FVector2D mouse){
        // display a tooltip describing the current item.
        // or could add as a child of the img widget
        //FWidgetData w = Game->myhud->widgets[ item.Type ];
        //Game->myhud->ui->tooltip->Set( w.Label + FString(": ") + w.Tooltip );
        //// put the tooltip as a child of the slot
        //slot->Add( tooltip );
        CostWidget* costWidget = Game->myhud->ui->costWidget;
        FUnitsDataRow data = Game->unitsData[ item.Type ];
        costWidget->Set( data.Name, data.GoldCost, data.LumberCost, data.StoneCost, data.Description );
        costWidget->hidden = 1;
        slot->Add( costWidget );

        // Set the costWidget's position based on the slot size.
        costWidget->Align = HotSpot::HCenter | HotSpot::OnTopOfParent;

        ///Game->myhud->ui->itemBelt->Add( costWidget );
        ///costWidget->Pos.Y = - costWidget->Size.Y - 8;
        return 0;
      };
    }
  }
  else
  {
    // No items in the belt.
    itemBelt->SetNumSlots(0, 0);
  }

  SlotPalette* abilities = Game->myhud->ui->rightPanel->abilities;
  // Things it can spawn. Make sure abilities size is at least right size for it.
  for( int i = 0; i < UnitsData.Spawns.Num(); i++ ) // DRAW SPAWNS (with costs)
  {
    Types type = UnitsData.Spawns[i];
    
    FWidgetData w = Game->myhud->widgets[ type ];
    SlotEntry* slot = abilities->SetSlotTexture( i, w.Icon );
    FUnitsDataRow ud = Game->unitsData[type];

    // Set the hover of this slot to show the cost of the spawn
    slot->OnHover = [ud](FVector2D mouse){
      Game->myhud->ui->costWidget->Set( ud.Name, ud.GoldCost, ud.LumberCost, ud.StoneCost, ud.Description );
      return 0;
    };
  }

  // Then render the abilities on the next line
  for( int i = 0; i < UnitsData.Abilities.Num(); i++ ) // abilities
  {
    Types type = UnitsData.Abilities[i];
    abilities->SetSlotTexture( UnitsData.Spawns.Num() + i,
      Game->myhud->widgets[ type ].Icon );
  }

  
}

FUnitsDataRow AUnit::GetTraits()
{
  FUnitsDataRow data = UnitsData;
  for( int i = 0; i < BonusTraits.size(); i++ )
    data += BonusTraits[i].traits;
  return data;
}

void AUnit::Tick( float t )
{
  Super::Tick( t );

  // Tick all the traits
  for( int i = BonusTraits.size() - 1; i >= 0; i-- ) {
    BonusTraits[i].time -= t;
    if( BonusTraits[i].time <= 0 )
      BonusTraits.erase( BonusTraits.begin() + i );
  }
}
