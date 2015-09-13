#include "Wryv.h"
#include "NotifyTreeFinishedFall.h"
#include "GlobalFunctions.h"

UNotifyTreeFinishedFall::UNotifyTreeFinishedFall( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  
}

bool UNotifyTreeFinishedFall::Received_Notify( USkeletalMeshComponent* MeshComp, UAnimSequence* AnimSeq ) const
{
  LOG( "NotifyTreeFinishedFall::Received_Notify() from Mesh=%s %d events", *MeshComp->GetName() );
  return 1;
}


