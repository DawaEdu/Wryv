#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "NotifyTreeFinishedFall.generated.h"

UCLASS()
class UNotifyTreeFinishedFall : public UAnimNotify
{
  GENERATED_BODY()
public:
  UNotifyTreeFinishedFall( const FObjectInitializer& PCIP );
  virtual bool Received_Notify( USkeletalMeshComponent* MeshComp, UAnimSequence* AnimSeq ) const;
};


