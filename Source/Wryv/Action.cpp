#include "Wryv.h"

#include "Action.h"
#include "GameObject.h"
#include "GlobalFunctions.h"

UAction::UAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
  Icon = 0;
}

void UAction::OnRefresh()
{
  info( FS( "Ability %s has refreshed", *Text ) );
}

void UAction::OnComplete()
{
  info( FS( "Action %s has completed", *Text ) );
}

void UAction::Step( float t )
{
  // Only step the counter if the cooldown isn't complete yet.
  if( !cooldown.Done() )
  {
    cooldown.Step( t );  //                                     <
    // When its done, hit OnRefresh() (only once, the first time |)
    if( cooldown.Done() )
      OnRefresh();
  }
}


