#include "RTSGame.h"
#include "PlayerControl.h"
#include "WidgetData.h"
#include "RTSGameInstance.h"

APlayerControl::APlayerControl( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  UE_LOG( LogTemp, Warning, TEXT("APlayerControl::APlayerControl") );
}

FSceneView* APlayerControl::GetSceneView(ULocalPlayer* LocalPlayer)
{
  // Create a view family for the game viewport
	FSceneViewFamilyContext ViewFamily( FSceneViewFamily::ConstructionValues(
		LocalPlayer->ViewportClient->Viewport, GetWorld()->Scene,
    LocalPlayer->ViewportClient->EngineShowFlags )
		.SetRealtimeUpdate(true) );

	// Calculate a view where the player is to update the
  // streaming from the players start location
	FVector ViewLocation;
	FRotator ViewRotation;
	FSceneView* SceneView = LocalPlayer->CalcSceneView( &ViewFamily, ViewLocation,
    ViewRotation, LocalPlayer->ViewportClient->Viewport );

  return SceneView;
}

bool APlayerControl::TraceMulti(const FVector2D ScreenPosition, vector<FHitResult>& HitResult)
{
	// Early out if we clicked on a HUD hitbox
	if( GetHUD() != NULL && GetHUD()->GetHitBoxAtCoordinates(ScreenPosition, true) )
	{
		return false;
	}
  
  ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if( LocalPlayer != NULL && LocalPlayer->ViewportClient != NULL &&
      LocalPlayer->ViewportClient->Viewport != NULL )
	{
    // Create a view family for the game viewport
	  FSceneViewFamilyContext ViewFamily( FSceneViewFamily::ConstructionValues(
		  LocalPlayer->ViewportClient->Viewport, GetWorld()->Scene,
      LocalPlayer->ViewportClient->EngineShowFlags )
		  .SetRealtimeUpdate(true) );

	  // Calculate a view where the player is to update the
    // streaming from the players start location
	  FVector ViewLocation;
	  FRotator ViewRotation;
	  FSceneView* SceneView = LocalPlayer->CalcSceneView( &ViewFamily, ViewLocation,
      ViewRotation, LocalPlayer->ViewportClient->Viewport );
		if( SceneView )
		{
			FVector origin, direction, endPt;
      SceneView->DeprojectFVector2D(ScreenPosition, origin, direction);
      endPt = origin + direction * 100000.f;
      UE_LOG( LogTemp, Warning, TEXT("sp: (%f, %f)\n"), ScreenPosition.X, ScreenPosition.Y );
      UE_LOG( LogTemp, Warning, TEXT("origin: (%f, %f, %f)\n"), origin.X, origin.Y, origin.Z );
      UE_LOG( LogTemp, Warning, TEXT("direction: (%f, %f, %f)\n"), endPt.X, endPt.X, endPt.Z );
      
      FCollisionQueryParams fqp("ClickableTrace", true);
      TArray<FHitResult> res;
      bool hit = GetWorld()->LineTraceMultiByChannel(res,
        origin, endPt, ECollisionChannel::ECC_EngineTraceChannel1, fqp );
      //UE_LOG( LogTemp, Warning, TEXT("-- Hit %d objects\n"), (int)res.Num() );

      for( int i = 0; i < res.Num(); i++ )
      {
        //UE_LOG( LogTemp, Warning, TEXT("-- Hit %s\n"), *res[i].Actor->GetName() );
        HitResult.push_back( res[i] );
      }
      return hit;
		}
	}
  
	return false;
}

bool APlayerControl::Trace(FVector2D ScreenPosition, AActor* actor, FHitResult& hit)
{
  // Early out if we clicked on a HUD hitbox
	if( GetHUD() != NULL && GetHUD()->GetHitBoxAtCoordinates(ScreenPosition, true) )
	{
		return false;
	}
  
  ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
	if( LocalPlayer != NULL && LocalPlayer->ViewportClient != NULL &&
      LocalPlayer->ViewportClient->Viewport != NULL )
	{
    // Create a view family for the game viewport
	  FSceneViewFamilyContext ViewFamily( FSceneViewFamily::ConstructionValues(
		  LocalPlayer->ViewportClient->Viewport, GetWorld()->Scene,
      LocalPlayer->ViewportClient->EngineShowFlags )
		  .SetRealtimeUpdate(true) );

	  // Calculate a view where the player is to update the
    // streaming from the players start location
	  FVector ViewLocation;
	  FRotator ViewRotation;
	  FSceneView* SceneView = LocalPlayer->CalcSceneView( &ViewFamily, ViewLocation,
      ViewRotation, LocalPlayer->ViewportClient->Viewport );
		if( SceneView )
		{
			FVector origin, direction, endPt;
      SceneView->DeprojectFVector2D(ScreenPosition, origin, direction);
      endPt = origin + direction * 100000.f;
      //UE_LOG( LogTemp, Warning, TEXT("sp: (%f, %f)\n"), ScreenPosition.X, ScreenPosition.Y );
      //UE_LOG( LogTemp, Warning, TEXT("origin: (%f, %f, %f)\n"), origin.X, origin.Y, origin.Z );
      //UE_LOG( LogTemp, Warning, TEXT("direction: (%f, %f, %f)\n"), endPt.X, endPt.Y, endPt.Z );
      
      FCollisionQueryParams fqp("ClickableTrace", true);
      bool h = actor->ActorLineTraceSingle( hit, origin, endPt,
        ECollisionChannel::ECC_EngineTraceChannel1, fqp );
      return h;
		}
	}
  
	return false;
}

