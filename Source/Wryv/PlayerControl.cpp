#include "Wryv.h"
#include "PlayerControl.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"
#include "WryvGameMode.h"
#include "GroundPlane.h"
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

FHitResult APlayerControl::TraceAgainst( AActor* actor, const FVector2D& ScreenPosition )
{
  FHitResult hit;
  ULocalPlayer* LP = GetLocalPlayer();
  if( !LP ) return hit;
  FSceneViewFamilyContext ViewFamily( FSceneViewFamily::ConstructionValues(
    LP->ViewportClient->Viewport, GetWorld()->Scene,
    LP->ViewportClient->EngineShowFlags ).SetRealtimeUpdate( true ) );
  FVector viewLoc;
  FRotator viewRotation;
  FSceneView* SceneView = LP->CalcSceneView( &ViewFamily, viewLoc, viewRotation, LP->ViewportClient->Viewport );

  if( !SceneView ) {
    error( "SceneView not ready" );
    return hit;
  }
  Ray ray;
  // Get the origin & direction of a ray that corresponds with ScreenPosition according to the sceneview.
  SceneView->DeprojectFVector2D( ScreenPosition, ray.start, ray.dir );
  ray.SetLen( 1e6f );
  FCollisionQueryParams fqp( "floor trace", true );
  actor->ActorLineTraceSingle( hit, ray.start, ray.end, ECollisionChannel::ECC_GameTraceChannel2, fqp );
  return hit;
}

FHitResult APlayerControl::TraceAgainst( AActor* actor, const FVector& eye, const FVector& lookDir )
{
  FHitResult hit;
  FCollisionQueryParams fcqp( "floor trace", true );
  FVector endPt = eye + lookDir*1e6f;
  if( !actor->ActorLineTraceSingle( hit, eye, endPt, ECollisionChannel::ECC_GameTraceChannel2, fcqp ) )
  {
    warning( FS( "Trace against %s failed", *actor->GetName() ) );
  }
  return hit;
}

FHitResult APlayerControl::PickClosest( const FVector2D& ScreenPosition )
{
  FHitResult hit;
  // Trace into the scene and check what got hit.
  Game->pc->GetHitResultAtScreenPosition( ScreenPosition, ECollisionChannel::ECC_GameTraceChannel2, true, hit );
  return hit;
}

FHitResult APlayerControl::PickClosest( const FVector& eye, const FVector& lookDir )
{
  FHitResult res;
  FVector end = eye + lookDir*1e6f;
  FCollisionQueryParams fcqp( "floor trace", true );
  // Returns closest hit
  FCollisionObjectQueryParams objectTypes = FCollisionObjectQueryParams( 
    FCollisionObjectQueryParams::InitType::AllObjects );
  GetWorld()->LineTraceSingleByObjectType( res, eye, end, objectTypes, fcqp );
  return res;
}

set<AGameObject*> APlayerControl::Pick( FVector pos, FCollisionShape shape )
{
  FCollisionQueryParams fqp;
  TArray<FOverlapResult> overlaps;
  FQuat quat( 0.f, 0.f, 0.f, 0.f );
  FCollisionObjectQueryParams objectTypes = FCollisionObjectQueryParams( 
    FCollisionObjectQueryParams::InitType::AllObjects );
  GetWorld()->OverlapMultiByObjectType( overlaps, pos, quat, objectTypes, shape, fqp );
  set<AGameObject*> intersections;
  for( int i = 0; i < overlaps.Num(); i++ )
    if( AGameObject* go = Cast<AGameObject>( overlaps[i].GetActor() ) )
      intersections.insert( go );
  return intersections;
} 

set<AGameObject*> APlayerControl::PickByCylinder( AGameObject* object )
{
  FCollisionShape c1 = object->GetBoundingCylinder();
  FCollisionQueryParams fqp;
  fqp.AddIgnoredActor( object ); // Don't report collisions with self.
  TArray<FOverlapResult> overlaps;
  FQuat quat( 0.f, 0.f, 0.f, 0.f );
  FCollisionObjectQueryParams objectTypes = FCollisionObjectQueryParams( 
    FCollisionObjectQueryParams::InitType::AllObjects );
  GetWorld()->OverlapMultiByObjectType( overlaps, object->Pos, quat, objectTypes, c1, fqp ); // default is to interpret as a line!
  
  set<AGameObject*> intersections;
  for( int i = 0; i < overlaps.Num(); i++ )
    if( AGameObject* go = Cast<AGameObject>( overlaps[i].GetActor() ) )
      intersections.insert( go );

  return intersections;
}

set<AGameObject*> APlayerControl::Pick( AGameObject* object )
{
  return PickExcept( object, {object} );
}

set<AGameObject*> APlayerControl::PickExcept( AGameObject* object, set<AGameObject*> except )
{
  FComponentQueryParams fqp;
  
  except.insert( object );

  // this will cause the object to pick itself if you don't put it in the except group
  for( AGameObject* o : except )
  {
    if( !o )
    {
      warning( "NULL object in PickExcept()" );
    }
    else
    {
      fqp.AddIgnoredActor( o );
    }
  }

  FQuat quat( 0.f, 0.f, 0.f, 0.f );
  FCollisionObjectQueryParams objectTypes = FCollisionObjectQueryParams( 
    FCollisionObjectQueryParams::InitType::AllObjects );
  objectTypes.AddObjectTypesToQuery( ECollisionChannel::ECC_GameTraceChannel3 ); // Channel 3 is
  // the RESOURCES channel.

  set<AGameObject*> intersections;

  // grab the hitBounds
  TArray<FOverlapResult> overlaps;

  FRotator ro = object->hitBounds->GetComponentRotation();
  FVector ve = object->hitBounds->GetComponentLocation();
  // We're looking on channel 4 ("CHECKERS" channel)
  // for objects sitting on CHANNEL 3 ("RESOURCES") channel.
  GetWorld()->ComponentOverlapMultiByChannel( overlaps, object->hitBounds, ve, ro,
    ECollisionChannel::ECC_GameTraceChannel4, fqp, objectTypes );
  for( int i = 0; i < overlaps.Num(); i++ )
    if( AGameObject* go = Cast<AGameObject>( overlaps[i].GetActor() ) )
      intersections.insert( go );
  return intersections;

  TArray<UActorComponent*> comps = object->GetComponents();
  for( UActorComponent* ac : comps )
  {
    if( UPrimitiveComponent* up = Cast<UPrimitiveComponent>( ac ) )
    {
      info( FS( "Tracing with primitive %s", *up->GetName() ) );
      if( up->GetCollisionEnabled() != ECollisionEnabled::NoCollision )
      {
        FRotator ro = up->GetComponentRotation();
        FVector ve = up->GetComponentLocation();
        // We're looking on channel 4 ("CHECKERS" channel)
        // for objects sitting on CHANNEL 3 ("RESOURCES") channel.
        GetWorld()->ComponentOverlapMultiByChannel( overlaps, up, ve, ro,
          ECollisionChannel::ECC_GameTraceChannel4, fqp, objectTypes );
        for( int i = 0; i < overlaps.Num(); i++ )
          if( AGameObject* go = Cast<AGameObject>( overlaps[i].GetActor() ) )
            intersections.insert( go );
      }
    }
  }

  // Non-empty means intns exists
  return intersections;
}

set<AGameObject*> APlayerControl::Pick( const FVector2D& ScreenPosition )
{
  set<AGameObject*> objects;
  
  ULocalPlayer* LP = GetLocalPlayer();
  if( !LP ) return objects;

  // This can't be refactored into a function because the FSceneViewFamilyContext
  // self-destructs completely when it goes out of scope.
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
  Ray ray;
  SceneView->DeprojectFVector2D( ScreenPosition, ray.start, ray.dir );
  ray.SetLen( 1e6f );
  FCollisionQueryParams fqp( "ClickableTrace", true );
  TArray<FHitResult> res;
  GetWorld()->LineTraceMultiByChannel( res, ray.start, ray.end, ECollisionChannel::ECC_GameTraceChannel2, fqp );
  for( int i = 0; i < res.Num(); i++ ) {
    if( AGameObject *go = Cast< AGameObject >( res[i].GetActor() ) )
      objects.insert( go );
  }
  return objects;
}

set<AGameObject*> APlayerControl::Pick( const FVector& eye, const FVector& lookDir )
{
  set<AGameObject*> objects;
  // Casts a ray into the scene
  FCollisionQueryParams fqp( "ClickableTrace", true );
  TArray<FHitResult> res;
  FVector end = eye + lookDir*1e6f;
  GetWorld()->LineTraceMultiByChannel( res, eye, end, ECollisionChannel::ECC_GameTraceChannel2, fqp );
  for( int i = 0; i < res.Num(); i++ ) {
    if( AGameObject *go = Cast< AGameObject >( res[i].GetActor() ) )
      objects.insert( go );
  }
  return objects;
}  

// If InTypes is EMPTY, then it picks any type
set<AGameObject*> APlayerControl::Pick( const FBox2DU& box )
{
  set<AGameObject*> objects;
  
  // A 0 area box picks with a ray from TL corner of the click.
  if( !box.GetArea() ) {
    // use a ray pick
    //info( FS( "Point %f %f using a ray", box.TL().X, box.TL().Y ) );
    return Pick( box.TL() );
  }

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
    //ray.Print( FS( "box select ray #%d", i ) );
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
  FConvexVolume selectionVolume( planes );
  
  // go through all world actors and see what is intersected
  //Game->flycam->ClearViz();
  for( Team* team : Game->gm->teams )
  {
    for( AGameObject* go : team->units )
    {
      if( go->IsPendingKill()   ||   go->Dead ) skip;
      FBox box = go->hitBounds->Bounds.GetBox();
      // Selection by mesh's bounding box is best.
      if( selectionVolume.IntersectBox( box.GetCenter(), box.GetExtent() ) )
      {
        objects.insert( go );
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


