#include "Wryv.h"

#include "GameObjects/Buildings/Building.h"
#include "GameObjects/Things/Items/Item.h"
#include "UI/UICommand/Command/UIItemActionCommand.h"
#include "UI/HotSpot/GamePanels/ItemBeltPanel.h"
#include "UI/HotSpot/Elements/IText.h"
#include "UE4/TheHUD.h"
#include "GameObjects/Units/Unit.h"
#include "UE4/WryvGameInstance.h"

#include "UI/UICommand/Command/UIItemActionCommand.h"

ItemBeltPanel::ItemBeltPanel( UTexture* bkg, int rows, int cols, FVector2D entrySize, FVector2D pad ) :
  SlotPanel( "itembelt", bkg, rows, cols, entrySize, pad )
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

void ItemBeltPanel::Set( vector<AGameObject*> objects )
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
      Populate<UUIItemActionCommand>( unit->CountersItems, 0 );
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

void ItemBeltPanel::render( FVector2D offset )
{
  if( hidden ) return;

  SlotPanel::render( offset );
}


