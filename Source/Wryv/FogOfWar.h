#pragma once

#include "GameObject.h"
#include "Box2DU.h"
#include "GameFramework/Actor.h"
#include "FogOfWar.generated.h"

UCLASS()
class WRYV_API AFogOfWar : public AActor
{
	GENERATED_UCLASS_BODY()
public:
  UCanvasRenderTarget2D* CRTFogOfWar; // : UTexture (doesn't work as a uproperty)
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = FogOfWar )  UStaticMeshComponent* FogMesh; // renders with material made from crtfogofwar
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = FogOfWar )  UMaterialInstance* FogInverterBase; // material instance
  UMaterialInstanceDynamic* FogInverter; // MID Cannot be selected from dropdown
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = FogOfWar )  FLinearColor ClearColor;
  FBox FloorBox; // 

  //AFogOfWar( const FObjectInitializer& PCIP );
  virtual void BeginPlay();
  void Init( FBox floorBox );
  UFUNCTION() void DrawFogOfWar( UCanvas* canvas, int32 Width, int32 Height );
  void Blot( UCanvas* canvas, AGameObject *go );
  virtual void Tick( float t ) override;
};
