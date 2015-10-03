#include "Wryv.h"

#include "Building.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "InProgressUnit.h"

UInProgressUnit::UInProgressUnit( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UInProgressUnit::Cancel()
{
  // returns money for training the unit
  if( !OwningBuilding )
  {
    error( "OwningBuilding not set" );
    return;
  }

  OwningBuilding->team->Refund( UnitType );
}

void UInProgressUnit::OnComplete()
{
  UAction::OnComplete();
}

void UInProgressUnit::Step( float t )
{
  
}