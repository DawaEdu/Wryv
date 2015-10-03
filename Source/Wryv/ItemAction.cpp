#include "Wryv.h"

#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Item.h"
#include "ItemAction.h"
#include "Unit.h"
#include "WryvGameInstance.h"

UItemAction::UItemAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Unit = 0;
}

void UItemAction::Click()
{
  // instantiate the item & use it.
  AItem* item = Game->Make<AItem>( ItemClass );
  item->Use( Unit );
}

void UItemAction::OnComplete()
{
  UAction::OnComplete();
  info( FS( "Action %s has completed", *Text ) );
}

