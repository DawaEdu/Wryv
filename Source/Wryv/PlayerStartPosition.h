#pragma once

#include "GameFramework/PlayerStart.h"

#include "AIProfile.h"
#include "PlayerStartPosition.generated.h"

UCLASS()
class WRYV_API APlayerStartPosition : public APlayerStart
{
  GENERATED_UCLASS_BODY()

  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Team )  int32 TeamNumber;
  // Specify the AI level used by this player.
  UPROPERTY( EditAnywhere, meta=(MetaClass="AIProfile"), Category = UnitProperties ) FStringClassReference aiLevel;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Cosmetics )  USkeletalMeshComponent* Mesh;

  UAIProfile* aiProfile;
  virtual void PostInitializeComponents() override;

};