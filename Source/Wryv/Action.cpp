#include "Wryv.h"

#include "Action.h"
#include "GameObject.h"
#include "GlobalFunctions.h"

UAction::UAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

void UAction::Go(AGameObject* go)
{
  Object = go;
  info( FS( "Using Action %s [%s] on %s", *Text,
    *GetEnumName( TEXT("Abilities"), Ability ), *Object->Stats.Name ) );
}

void UAction::OnRefresh()
{
  info( FS( "Ability %s has refreshed", *Text ) );
}

void UAction::OnComplete()
{
  info( FS( "%s has completed action %s has completed", 
    *Object->Stats.Name, *Text ) );
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


