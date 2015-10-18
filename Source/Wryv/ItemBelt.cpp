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
  
  // Consume all of MouseUp, MouseDown events to prevent clickthru to other ui elts
  OnMouseUpLeft = []( FVector2D mouse ) {
    info( "Mouse upleft in itembelt" );
    return Consumed;
  };

  OnMouseDownLeft = []( FVector2D mouse ) {
    info( "Mouse downleft in itembelt" );
    return Consumed;
  };

  AbsorbsMouseUp = 1; // Down clicks on this elt won't fire mouse up events
}

void ItemBelt::ClearTextures()
{
  // Clears the textures inside the belt, so no references to texes that aren't
  // being used exist
  for( int i = 0; i < children.size(); i++ )
    ((Clock*)children[i])->Tex = 0;
}

void ItemBelt::Set( AGameObject* go )
{
  ClearTextures(); // Clear textures from previously selected object
  HideChildren();  // hide all clocks

  if( AUnit* unit = Cast<AUnit>( go ) )
  {
    // repopulate the # grid slots according to # items unit has
    // Populate the toolbelt, etc
    if( unit->CountersItems.Num() > children.size() )
    {
      error( FS( "Unit %s has %d items, but the itembelt maxes out @ %d items",
        *unit->GetName(), unit->CountersItems.Num(), GetNumActiveSlots() ) );
      Resize( 1, unit->CountersItems.Num() );
    }

    // Correct associated unit with the Counters.
    for( int i = 0; i < unit->CountersItems.Num(); i++ )
    {
      //info( FS( "initial: %s/%s",
      //  *unit->CountersItems[i]->AssociatedUnit->GetName(), 
      //  *unit->CountersItems[i]->AssociatedUnitName ) );
      unit->CountersItems[i]->AssociatedUnit = unit;
      unit->CountersItems[i]->AssociatedUnitName = unit->GetName();
      //info( FS( "After correcting AssociatedUnit: %s/%s",
      //  *unit->CountersItems[i]->AssociatedUnit->GetName(), 
      //  *unit->CountersItems[i]->AssociatedUnitName ) );
    }

    Show();
    SetNumSlots( 1, unit->CountersItems.Num() );
    Populate<UItemAction>( unit->CountersItems, 0 );
  }
  else if( ABuilding* building = Cast<ABuilding>( go ) )
  { 
    // building doesn't have any objects that go into itembelt yet.
    Show();
  }
  else
  {
    // otherwise leave itembelt empty with hidden children
    Hide();
  }
}

void ItemBelt::render( FVector2D offset )
{
  SlotPalette::render( offset );
}


