#include "RTSGame.h"
#include "CameraMan.h"

void ACameraMan::UpdateCamera( float DeltaTime )
{
  FString fs = FString::Printf( TEXT( "%f" ), DeltaTime );
  //UE_LOG( LogTemp, Warning, TEXT("%s"), fs );
  UE_LOG( LogTemp, Warning, TEXT("%f"), DeltaTime );
}


