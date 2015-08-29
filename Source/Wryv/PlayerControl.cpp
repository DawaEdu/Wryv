#include "Wryv.h"
#include "PlayerControl.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"
#include "WryvGameMode.h"
#include "GameObject.h"
#include "FlyCam.h"
#include "Runtime/Core/Public/Math/Plane.h"

APlayerControl::APlayerControl( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  LOG( "APlayerControl::APlayerControl()" );
  PrimaryActorTick.bCanEverTick = 1;
  PrimaryActorTick.bTickEvenWhenPaused = 1;
}

void APlayerControl::SetupInputComponent()
{
  PlayerInput->AddActionMapping( FInputActionKeyMapping( "KKey", EKeys::K, 0, 0, 0, 0 ) );
  static const FName InputComponentName( TEXT( "PlayerControllerInputComponent" ) );
  InputComponent = NewObject<UInputComponent>( this, InputComponentName );
  //InputComponent->BindAction( TEXT("KKey"), EInputEvent::IE_Pressed, this, &APlayerControl::K );
}

void APlayerControl::SetupInactiveStateInputComponent( UInputComponent* InComponent )
{
  LOG( "APlayerControl::SetupPlayerInputComponent() has %d components", CurrentInputStack.Num() );
}

ULocalPlayer* APlayerControl::GetLocalPlayer()
{
  ULocalPlayer* LP = Cast<ULocalPlayer>( Player );
  if( !LP || !LP->ViewportClient || !LP->ViewportClient->Viewport )
    return NULL; // LocalPlayer not ready, so return NULL
  return LP;
}

bool APlayerControl::Trace( const FVector2D& ScreenPosition, AActor* actor, FHitResult& hit )
{
  ULocalPlayer* LP = GetLocalPlayer();
  if( !LP ) return NULL;
  FSceneViewFamilyContext ViewFamily( FSceneViewFamily::ConstructionValues(
    LP->ViewportClient->Viewport, GetWorld()->Scene,
    LP->ViewportClient->EngineShowFlags ).SetRealtimeUpdate( true ) );
  FVector viewLoc;
  FRotator viewRotation;
  FSceneView* SceneView = LP->CalcSceneView( &ViewFamily, viewLoc, viewRotation, LP->ViewportClient->Viewport );

  if( !SceneView ) {
    error( "SceneView not ready" );
    return 0;
  }
  Ray ray;
  // Get the origin & direction of a ray that corresponds with ScreenPosition according to the sceneview.
  SceneView->DeprojectFVector2D( ScreenPosition, ray.start, ray.dir );
  ray.SetLen( 1e6f );
  FCollisionQueryParams fqp( "Click-Trace", true );
  return actor->ActorLineTraceSingle( hit, ray.start, ray.end, ECollisionChannel::ECC_GameTraceChannel9, fqp );
}

bool APlayerControl::TraceMulti( const FVector2D& ScreenPosition, vector<FHitResult>& HitResult )
{
  ULocalPlayer* LP = GetLocalPlayer();
  if( !LP ) return NULL;
  FSceneViewFamilyContext ViewFamily( FSceneViewFamily::ConstructionValues(
    LP->ViewportClient->Viewport, GetWorld()->Scene,
    LP->ViewportClient->EngineShowFlags ).SetRealtimeUpdate( true ) );
  FVector viewLoc;
  FRotator viewRotation;
  FSceneView* SceneView = LP->CalcSceneView( &ViewFamily, viewLoc, viewRotation, LP->ViewportClient->Viewport );
  if( !SceneView ) {
    error( "SceneView not ready" );
    return 0;
  }
  Ray ray;
  SceneView->DeprojectFVector2D( ScreenPosition, ray.start, ray.dir );
  ray.SetLen( 1e6f );
  FCollisionQueryParams fqp( "ClickableTrace", true );
  TArray<FHitResult> res;
  bool didHit = GetWorld()->LineTraceMultiByChannel( res, ray.start, ray.end, ECollisionChannel::ECC_GameTraceChannel9, fqp );
  for( int i = 0; i < res.Num(); i++ )
    HitResult.push_back( res[i] );
  return didHit;
}

set<AGameObject*> APlayerControl::PickWithType( const FBox2DU& box, set<Types> inTypes )
{
  return Pick( box, inTypes, {} );
}

set<AGameObject*> APlayerControl::PickButNotType( const FBox2DU& box, set<Types> notTypes )
{
  return Pick( box, {}, notTypes );
}

// If InTypes is EMPTY, then it picks any type
set<AGameObject*> APlayerControl::Pick( const FBox2DU& box, set<Types> inTypes, set<Types> notTypes )
{
  set<AGameObject*> objects;
  if( box.Empty() ) return objects;
  ULocalPlayer* LP = GetLocalPlayer();
  if( !LP ) return objects;
  FSceneViewFamilyContext ViewFamily( FSceneViewFamily::ConstructionValues(
    LP->ViewportClient->Viewport, GetWorld()->Scene,
    LP->ViewportClient->EngineShowFlags ).SetRealtimeUpdate( true ) );
  FVector viewLoc;
  FRotator viewRotation;
  FSceneView* SceneView = LP->CalcSceneView( &ViewFamily, viewLoc, viewRotation, LP->ViewportClient->Viewport );
  if( !SceneView ) {
    error( "SceneView not ready" );
    return objects;
  }

  vector<FVector2D> pts = { box.TL(), box.BL(), box.BR(), box.TR() };
  vector<Ray> rays;

  // get the 4 rays of the frustum
  for( int i = 0; i < pts.size(); i++ )
  {
    Ray ray;
    SceneView->DeprojectFVector2D( pts[i], ray.start, ray.dir );
    ray.SetLen( 1e4f );
    ray.Print( FS( "box select ray #%d", i ) );
    rays.push_back( ray );

    //Game->flycam->Visualize( ray.start, 1.f, FLinearColor::Green );
    //Game->flycam->Visualize( ray.end, 64.f, FLinearColor::Green );
  }

  // Make a frustum with the 6 planes formed by the 8 points
  TArray<FPlane, TInlineAllocator<6>> planes;
  planes.Push( FPlane( rays[0].end,   rays[0].start, rays[1].start ) ); // Left
  planes.Push( FPlane( rays[1].end,   rays[1].start, rays[2].start ) ); // Left
  planes.Push( FPlane( rays[2].end,   rays[2].start, rays[3].start ) ); // Right
  planes.Push( FPlane( rays[3].end,   rays[3].start, rays[0].start ) ); // Top
  planes.Push( FPlane( rays[1].start, rays[0].start, rays[2].start ) ); // Near
  planes.Push( FPlane( rays[2].end,   rays[0].end,   rays[1].end   ) ); // Far (CCW)
  FConvexVolume v( planes );
  
  // go through all world actors and see what is intersected
  //Game->flycam->ClearViz();
  for( Team* team : Game->gm->teams )
  {
    for( AGameObject* go : team->units )
    {
      //FBox box = go->GetComponentsBoundingBox();
      Types type = go->Stats.Type.GetValue();
      if( !go->IsPendingKill()   &&
           go != Game->flycam->floor   &&
           v.IntersectSphere( go->Pos, go->GetBoundingRadius() )  &&
         //v.IntersectBox( box.GetCenter(), box.GetExtent() ) )  &&
          // If the inTypes collection is specified, check it
          (!(inTypes.size()) || in( inTypes, type )) &&
          !in( notTypes, type )
       )
      {
        objects.insert( go );
        //AGameObject * viz = Game->Make<AGameObject>( Types::UNITCUBE, box.GetCenter(), box.GetSize() );
        //Game->flycam->viz.push_back( viz );
      }
    }
  }
  return objects;
}

bool APlayerControl::IsKeyDown( FKey key )
{
  //return GetInputKeyTimeDown( key ) != 0.f;
  return IsInputKeyDown( key );
}

bool APlayerControl::IsAnyKeyDown( vector<FKey> keys )
{
  bool d = 0;
  for( int i = 0; i < keys.size(); i++ )
    d |= IsInputKeyDown( keys[i] );
  return d;
}

void APlayerControl::Tick( float t )
{
  Super::Tick( t );

  // Tick Player Input as if unpaused, regardless
  TickPlayerInput( t, 0 );
}


