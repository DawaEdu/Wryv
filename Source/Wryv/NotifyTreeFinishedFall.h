#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "NotifyTreeFinishedFall.generated.h"

UCLASS()
class UNotifyTreeFinishedFall : public UAnimNotify
{
  GENERATED_UCLASS_BODY()
  virtual bool Received_Notify( USkeletalMeshComponent* MeshComp, UAnimSequence* AnimSeq ) const;
};


