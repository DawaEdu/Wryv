#include "RTSGame.h"
#include "Unit.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "PlayerControl.h"
#include "MyHUD.h"
#include "Widget.h"

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

  //Items.Push( Types::ITEMDEFENSEUP );

  if( Items.Num() )
  {
    // Move selection cursor to this unit.
    // Populate the toolbelt, etc
    int itemRows = 1 + ((Items.Num() - 1) / 4); // 1 + ( 5 - 1 )/4
    int itemCols = 4;
    itemCols = Items.Num() % 4;
    if( !itemCols )  itemCols = 4; // 

    vector<ImageWidget*> slots = Game->myhud->itemBelt->SetNumSlots( itemRows, itemCols );
  
    // The function associated with the Item is hooked up here.
    // Inventory size dictates #items.
    for( int i = 0; i < slots.size(); i++ )
    {
      ImageWidget *img = slots[i];
      Types item = Items[i];
      FWidgetData &data = Game->myhud->widgets[ item ];
      slots[i]->Icon = data.Icon;
      slots[i]->OnClicked = [this,i](){
        // use the item.
        this->ApplyEffect( Items[i] );
      };
      slots[i]->OnHover = [item](){
        // display a tooltip describing the current item.
        Game->myhud->tooltip->SetText( Game->myhud->widgets[ item ].Label );
      };
    }
  }
  else
  {
    // No items in the belt.
    Game->myhud->itemBelt->SetNumSlots(0, 0);
  }

  // Things it can spawn. Make sure useSlots size is at least right size
  // for 
  for( int i = 0; i < UnitsData.Spawns.Num(); i++ ) // DRAW SPAWNS (with costs)
  {
    Types type = UnitsData.Spawns[i];
    //Game->myhud->rightPanel->useSlots->SetSlot( i, Game->myhud->widgets[ type ].Icon );
  }

  // Then render the abilities on the next line
  for( int i = 0; i < UnitsData.Abilities.Num(); i++ ) // abilities
  {
    Types type = UnitsData.Abilities[i];
    //Game->myhud->rightPanel->useSlots->SetSlot( UnitsData.Spawns.Num() + i,
    //  Game->myhud->widgets[ type ].Icon );
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
