#pragma once

#include "GameFramework/WorldSettings.h"
#include "Settings.generated.h"

UCLASS()
class RTSGAME_API ASettings : public AWorldSettings
{
  GENERATED_BODY()
public:
  ASettings( const FObjectInitializer& PCIP );

};
