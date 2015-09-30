#include "Wryv.h"

#include "CastSpellAction.h"
#include "GameObject.h"
#include "GlobalFunctions.h"

UCastSpellAction::UCastSpellAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UCastSpellAction::Go(AGameObject* go)
{
  UAction::Go( go );
}

void UCastSpellAction::OnComplete()
{
  UAction::OnComplete();
}

