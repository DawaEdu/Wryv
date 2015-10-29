#pragma once

#include "GameFramework/Actor.h"
#include "GameObject.h"
#include "Shape.generated.h"

// Checkers & UI objects (gen. unintersectable)

UCLASS()
class WRYV_API AShape : public AGameObject
{
  GENERATED_UCLASS_BODY()
public:
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  UStaticMeshComponent* Mesh;
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Properties)  FString text;

  virtual void PostInitializeComponents() override;
  virtual bool SetDestination( FVector d );
};
