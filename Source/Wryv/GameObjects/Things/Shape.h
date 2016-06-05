#pragma once

#include "GameFramework/Actor.h"
#include "GameObjects/GameObject.h"
#include "Shape.generated.h"

// Checkers & UI objects (gen. unintersectable)

UCLASS()
class WRYV_API AShape : public AGameObject
{
  GENERATED_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UStaticMeshComponent* Mesh;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  FString text;
  
  AShape( const FObjectInitializer & PCIP );
  virtual void PostInitializeComponents() override;
  virtual bool SetDestination( FVector d );
};
