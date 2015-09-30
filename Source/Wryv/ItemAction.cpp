#include "Wryv.h"

#include "GameObject.h"
#include "GlobalFunctions.h"
#include "ItemAction.h"

UItemAction::UItemAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UItemAction::Go(AGameObject* go)
{
  UAction::Go( go );
}

void UItemAction::OnComplete()
{
  UAction::OnComplete();
  info( FS( "Action %s has completed", *Text ) );
}

