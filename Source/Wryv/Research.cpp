#include "Wryv.h"

#include "GameObject.h"
#include "Team.h"
#include "Research.h"

UResearch::UResearch( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UResearch::Go(AGameObject* go)
{
  UAction::Go( go );  // Call base
  team = go->team;    // save the team for completion phase
}

void UResearch::OnComplete()
{
  // Tell team that research is complete
  UAction::OnComplete();
  team->Researches.push_back( this );
}


