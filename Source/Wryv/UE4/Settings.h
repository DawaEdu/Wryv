#pragma once

#include "GameFramework/WorldSettings.h"
#include "Settings.generated.h"

UCLASS()
class WRYV_API ASettings : public AWorldSettings
{
  GENERATED_BODY()
public:
  ASettings( const FObjectInitializer& PCIP );
  virtual void NotifyBeginPlay();

};
