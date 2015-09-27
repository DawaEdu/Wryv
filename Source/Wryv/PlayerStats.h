#pragma once

#include "PlayerStats.generated.h"

class AProjectile;

UCLASS( config=Game, BlueprintType, Blueprintable, meta=(ShortTooltip="Represents a player") )
class WRYV_API UPlayerStats : public UObject
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
  int32 aiLevel;
  // If food% is 40/60 (66%) then perhaps we want to build another farm.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
  float foodFraction;
  // Attacks are periodic. We scout at intervals.
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = AI)
  float scoutInterval;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Attack, meta=(MetaClass="Projectile") )
  TSubclassOf<AProjectile> Projectile;
  //UClass* uclass;
  
  UPROPERTY( EditAnywhere, Category = Attack, meta=(MetaClass="Projectile") )
  FStringClassReference ProjectileType;

  UPlayerStats( const FObjectInitializer& PCIP );
  UFUNCTION(BlueprintCallable, Category = AI)
  FString ToString();
};


