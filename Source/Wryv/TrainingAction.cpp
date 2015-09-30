#include "Wryv.h"

#include "TrainingAction.h"
#include "GameObject.h"
#include "GlobalFunctions.h"

UTrainingAction::UTrainingAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UTrainingAction::Go(AGameObject* go)
{
  UAction::Go( go );
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
  int index = removeElement( Object->CountersTraining, this );
}

