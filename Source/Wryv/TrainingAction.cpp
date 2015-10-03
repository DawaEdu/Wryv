#include "Wryv.h"

#include "Building.h"
#include "GameObject.h"
#include "GlobalFunctions.h"

#include "InProgressUnit.h"
#include "TrainingAction.h"

UTrainingAction::UTrainingAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UTrainingAction::Click(ABuilding* building)
{
  Building = building;
  
  // Construct an in-progress counter & add it to counters for this object
  UInProgressUnit* unitInProgress = NewObject<UInProgressUnit>( 
    this, UInProgressUnit::StaticClass() );
  unitInProgress->UnitType = UnitType; // Class of the unit that is being created
  unitInProgress->OwningBuilding = Building; // Cancellation req goes thru to building owning icon
  unitInProgress->Icon = Icon; // same icon as button to generate unit
  
  Building->CountersInProgress.push_back( unitInProgress );
}

void UTrainingAction::OnRefresh()
{
  UAction::OnRefresh();
  // When a training action hits refresh, it actually completes
  OnComplete();
}

void UTrainingAction::OnComplete()
{
  UAction::OnComplete();
  // remove the training action from the original object's queue
  int index = removeElement( Building->CountersTraining, this );
}

