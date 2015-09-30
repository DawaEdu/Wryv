#include "Wryv.h"

#include "BuildAction.h"
#include "GameObject.h"
#include "GlobalFunctions.h"

UBuildAction::UBuildAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UBuildAction::Go(AGameObject* go)
{
  UAction::Go( go );
}

void UBuildAction::OnComplete()
{
  UAction::OnComplete();
}

