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

  // Move selection cursor to this unit.
  // Populate the toolbelt, etc
  int itemRows = Items.Num() / 4;
  int itemCols = Items.Num() % 4;
  vector<ImageWidget*> slots = Game->myhud->itemSlots->SetNumSlots( itemRows, itemCols );
  
  // The function associated with the Item is hooked up here.
  // Inventory size dictates #items.
  for( int i = 0; i < slots.size(); i++ )
  {
    ImageWidget *img = slots[i];
    Types item = Items[i];
    slots[i]->Icon = Game->myhud->widgets[ Items[i] ].Icon;

    slots[i]->OnClicked = [this,i](){
      // use the item.
      this->ApplyEffect( Items[i] );
    };
    slots[i]->OnHover = [item](){
      // display a tooltip describing the current item.
      Game->myhud->tooltip->SetText( Game->myhud->widgets[ item ].Label );
    };
  }

  Items.Push( Types::ITEMDEFENSEUP );

  // Things it can spawn.
  if( UnitsData.Spawns.Num() ) // DRAW SPAWNS (with costs)
  {
    //DrawText( TEXT("Spawns"), FLinearColor::White, x, yPos );
    //DrawGroup( lastClickedObject->UnitsData.Spawns, x, yPos += BarSize, IconSize.X, BarSize, 1, 1, 1 );
    // go thru all the unitsdata spawns and 
    //Game->myhud->spawnQueue->SetNumSlots( 

    
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
