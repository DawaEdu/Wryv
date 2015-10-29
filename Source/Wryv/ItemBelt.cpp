#include "Wryv.h"

#include "Building.h"
#include "Item.h"
#include "ItemAction.h"
#include "ItemBelt.h"
#include "ITextWidget.h"
#include "TheHUD.h"
#include "Unit.h"
#include "WryvGameInstance.h"

#include "ItemAction.h"

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

  // Add additional text for qty to each clock
  for( int i = 0; i < GetNumChildren(); i++ )
  {
    TextWidget* qty = new TextWidget( "1" );
    qty->Align = Alignment::BottomRight;
    GetChild(i)->Add( qty );
  }
}

void ItemBelt::Set( vector<AGameObject*> objects )
{
  Blank(); // Clear textures from previously selected object
  HideChildren();  // Hide all clocks
  Hide();  // Hide the belt itself
  if( !objects.size() ) return;

  for( int i = 0; i < objects.size(); i++ )
  {
    AGameObject* go = objects[i];
    if( AUnit* unit = Cast<AUnit>( go ) )
    {
      // repopulate the # grid slots according to # items unit has
      // Populate the toolbelt, etc
      if( unit->CountersItems.Num() > GetNumChildren() )
      {
        error( FS( "Unit %s has %d items, but the itembelt maxes out @ %d items",
          *unit->GetName(), unit->CountersItems.Num(), GetNumActiveSlots() ) );
        Resize( 1, unit->CountersItems.Num() );
      }
    
      Show();
      SetNumSlots( 1, unit->CountersItems.Num() );
      Populate<UItemAction>( unit->CountersItems, 0 );
      
    }
    else if( ABuilding* building = Cast<ABuilding>( go ) )
    { 
      // building doesn't have any objects that go into itembelt yet.
    }
    else
    {
      // otherwise leave itembelt empty with hidden children
    }
  }
}

void ItemBelt::render( FVector2D offset )
{
  if( hidden ) return;

  SlotPalette::render( offset );
}


