#include "Wryv.h"

#include "BuildInProgress.h"
#include "GameObject.h"
#include "GlobalFunctions.h"

UBuildInProgress::UBuildInProgress( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UBuildInProgress::Go(AGameObject* go)
{
  UAction::Go( go );
}

void UBuildInProgress::OnComplete()
{
  UAction::OnComplete();
}

