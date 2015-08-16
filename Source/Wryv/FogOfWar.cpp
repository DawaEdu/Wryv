#include "Wryv.h"
#include "FogOfWar.h"
#include "FlyCam.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"
#include "TheHUD.h"
#include "GameObject.h"

AFogOfWar::AFogOfWar( const FObjectInitializer& PCIP )
{
  PrimaryActorTick.bCanEverTick = 1;
  CRTFogOfWar = 0;
  FogInverter = 0;
  FogMesh = PCIP.CreateDefaultSubobject<UStaticMeshComponent>( this, "Fog Mesh" );
}

// we'll start with the fogofwar covering the entire scene.
// then minimize its size to increase resolution
void AFogOfWar::BeginPlay()
{
  LOG( "AFogOfWar::BeginPlay()" );
  CRTFogOfWar = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D( GetWorld(),
    UCanvasRenderTarget2D::StaticClass(), 128, 128 );
  CRTFogOfWar->OnCanvasRenderTargetUpdate.AddDynamic( this, &AFogOfWar::DrawFogOfWar );
  //CRTFogOfWar->ClearColor = FLinearColor::Black;
  CRTFogOfWar->ClearColor = FLinearColor( 0,0,0,.85f );
  
  // Create an instanced copy of the FogInverterBase and feed it 
  FogInverter = UMaterialInstanceDynamic::Create( FogInverterBase, this );
  FogInverter->SetTextureParameterValue( FName( "TEXTURE" ), CRTFogOfWar );

  FogMesh->SetMaterial( 0, FogInverter );
}

void AFogOfWar::Init( FBox floorBox )
{
  FloorBox = floorBox;
  SetActorScale3D( FloorBox.GetSize() );
  FVector Center = FloorBox.GetCenter();
  Center.Z = FloorBox.Max.Z; // 
  SetActorLocation( Center ) ;
}

void AFogOfWar::DrawFogOfWar( UCanvas* canvas, int32 Width, int32 Height )
{
  // This renders the fog of war into the canvas. I'm doing it as
  // sprites.
  //   1. render fogBlots at each unit's location
  //   2. render the max(1-fogBlots,1.f) texture on top of the scene

  if( !CRTFogOfWar || !Game->hud || !Game->hud->WarBlot )  return;
  //LOG( "DrawFogOfWar" );
  //CRTFogOfWar->ClearColor = ClearColor;// FLinearColor( 0.15f, 0.15f, 0.15f, 0.15f );

  // The location of the fogBlots texture is going to be actually just on top of the terrain.
  // It will move with the camera (attached to the FlyCam object). We'll use a small quad instead
  // of a very large quad so that the texture resolution is good.
  FVector2D CanvasSize( Width, Height );
  FBox floorBox = Game->flycam->floorBox;
  FVector2D floorBoxSize( floorBox.GetSize().X, floorBox.GetSize().Y );
  FVector2D floorOrigin( floorBox.Min.X, floorBox.Min.Y );
  float r = 25.f;
  
  // Just use the X value (width) (or possibly maximum extent) to find the worldScale
  for( int i = 0; i < Game->gm->playersTeam->units.size(); i++ )
  {
    AGameObject *go = Game->gm->playersTeam->units[i];
    FVector2D pos( go->Pos.X, go->Pos.Y );
    FVector2D unitizedPos = (pos - floorOrigin) / floorBoxSize;
    float radiusUnitized = 0.25f; //go->Stats.SightRange / floorBoxSize.X;
    unitizedPos -= FVector2D( radiusUnitized, radiusUnitized );
    FVector2D blot = unitizedPos * CanvasSize;
    float radiusPX = radiusUnitized * CanvasSize.X;
    //LOG( "Blot @ (%f %f) radius (%f)", blot.X, blot.Y, radiusPX );
    Game->hud->DrawMaterial( canvas, Game->hud->WarBlot, blot.X, blot.Y, radiusPX, radiusPX, 0, 0, 1, 1 );
  }

}

void AFogOfWar::Blot( UCanvas* canvas, AGameObject *go )
{
  
}

void AFogOfWar::Tick( float t )
{
  Super::Tick( t );
  
  CRTFogOfWar->UpdateResource();
}




