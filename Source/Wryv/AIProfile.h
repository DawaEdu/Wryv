#pragma once

#include "Wryv.h"
#include "AI.h"
#include "AIProfile.generated.h"
//
// Wrapper class to make the FAI object blueprintable. We want the object
// to be A USTRUCT() so that it fits inside the Team object (without the Team object having
// to be a UCLASS() as well).
// The artificial intelligence for the team. Parameters for the AI.
UCLASS( BlueprintType, Blueprintable, meta=(ShortTooltip="An AI profile") )
class WRYV_API UAIProfile : public UObject
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
  TEnumAsByte<AILevel> aiLevel;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
  FAI ai;
  UAIProfile( const FObjectInitializer& PCIP );
  UFUNCTION(BlueprintCallable, Category = AI)
  FString ToString();
};


