#include "Wryv.h"
#include "Goldmine.h"

AGoldmine::AGoldmine( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  EntryPoint = PCIP.CreateDefaultSubobject<USceneComponent>( this, TEXT( "entryPt" ) );
  EntryPoint->AttachTo( GetRootComponent() );
}

void AGoldmine::PostInitializeComponents()
{
  Super::PostInitializeComponents();
}

FVector AGoldmine::GetEntryPoint()
{
  FVector p = EntryPoint->GetComponentLocation();
  return p;
}


