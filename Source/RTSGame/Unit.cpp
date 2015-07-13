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

void AUnit::ApplyEffect( Types type )
{
  // timeLength, dataSet
  FUnitsDataRow dataSet = Game->unitsData[ type ];
  UE_LOG( LogTemp, Warning, TEXT("Applying %s for %f seconds"),
    *dataSet.Name, dataSet.TimeLength );

  BonusTraits.push_back( PowerUpTimeOut( dataSet.TimeLength, dataSet ) );
}

void AUnit::OnSelected()
{
  AGameObject::OnSelected();

  if( Items.Num() )
  {
    // Move selection cursor to this unit.
    // Populate the toolbelt, etc
    int itemRows = 1 + ((Items.Num() - 1) / 4); // 1 + ( 5 - 1 )/4
    int itemCols = 4;
    itemCols = Items.Num() % 4;
    if( !itemCols )  itemCols = 4; // 

    vector<ImageWidget*> slots = Game->myhud->ui->itemBelt->SetNumSlots( itemRows, itemCols );
    
    // The function associated with the Item is hooked up here.
    // Inventory size dictates #items.
    for( int i = 0; i < slots.size(); i++ )
    {
      ImageWidget *img = slots[i];
      Types item = Items[i];
      FWidgetData &data = Game->myhud->widgets[ item ];
      slots[i]->Icon = data.Icon;
      slots[i]->OnClicked = [this,i](FVector2D mouse){
        // use the item.
        this->ApplyEffect( Items[i] );
      };
      slots[i]->OnHover = [img,item,data](FVector2D mouse){
        // display a tooltip describing the current item.
        // or could add as a child of the img widget
        FWidgetData w = Game->myhud->widgets[ item ];

        // 
        Game->myhud->ui->tooltip->Set( w.Label + FString(": ") + w.Tooltip );
        img->Add( Game->myhud->ui->tooltip );

        //CostWidget *costWidget = Game->myhud->ui->costWidget;
        //FUnitsDataRow data = Game->unitsData[ item ];
        //costWidget->Set( data.Name, data.GoldCost, data.LumberCost, data.StoneCost, data.Description );
      };
    }
  }
  else
  {
    // No items in the belt.
    Game->myhud->ui->itemBelt->SetNumSlots(0, 0);
  }

  Game->myhud->ui->rightPanel->abilities->ResetSlots();
  // Things it can spawn. Make sure abilities size is at least right size for it.
  SlotPalette *sp = Game->myhud->ui->rightPanel->abilities;
  for( int i = 0; i < UnitsData.Spawns.Num(); i++ ) // DRAW SPAWNS (with costs)
  {
    Types type = UnitsData.Spawns[i];
    
    FWidgetData w = Game->myhud->widgets[ type ];
    ImageWidget* slot = sp->SetSlot( i, w.Icon );
    FUnitsDataRow ud = Game->unitsData[type];

    // Set the hover of this slot to show the cost of the spawn
    slot->OnHover = [ud](FVector2D mouse){
      Game->myhud->ui->costWidget->Set( ud.Name, ud.GoldCost, ud.LumberCost, ud.StoneCost, ud.Description );
    };
  }

  // Then render the abilities on the next line
  for( int i = 0; i < UnitsData.Abilities.Num(); i++ ) // abilities
  {
    Types type = UnitsData.Abilities[i];
    sp->SetSlot( UnitsData.Spawns.Num() + i,
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
