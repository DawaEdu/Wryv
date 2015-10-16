#include "Wryv.h"

#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "GroundPlane.h"
#include "PlayerControl.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
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
  InputComponent = NewObject<UInputComponent>( GetTransientPackage(), InputComponentName );

  // Read collision profiles
  TArray< TSharedPtr< FName > > profileNames;
  UCollisionProfile::GetProfileNames( profileNames );
  for( int i = 0; i < profileNames.Num(); i++ )
  {
    if( !profileNames[i].Get() )  skip;

    FName profile = *profileNames[i].Get();
    ECollisionChannel channel;
    FCollisionResponseParams fcrp;
    UCollisionProfile::GetChannelAndResponseParams( profile, channel, fcrp );

    LOG( "COLLISION PROFILE %s ON CHANNEL [ %d ]", *profile.ToString(), (int)channel );
    CollisionChannels[ profile.ToString() ]   =   channel;
  }
}

void APlayerControl::SetupInactiveStateInputComponent( UInputComponent* InComponent )
{
  LOG( "APlayerControl::SetupPlayerInputComponent() has %d components", CurrentInputStack.Num() );
}

FHitResult APlayerControl::TraceAgainst( UPrimitiveComponent* component, const FVector2D& ScreenPosition )
{
  Ray ray;
	if( !UGameplayStatics::DeprojectScreenToWorld( this, ScreenPosition, ray.start, ray.dir ) )
	{
    error( FS( "Could not DeprojectScreenToWorld (%f %f)", ScreenPosition.X, ScreenPosition.Y ) );
  }
  ray.SetLen( 1e6f );
  
  return TraceAgainst( component, ray );
}

FHitResult APlayerControl::TraceAgainst( UPrimitiveComponent* component, const Ray& ray )
{
  FHitResult hit;
  FCollisionQueryParams fcqp( true );
  if( !component->LineTraceComponent( hit, ray.start, ray.end, fcqp ) )
  {
    //warning( FS( "TraceAgainst (%s) failed %f %f %f => %f %f %f",
    //  *component->GetName(), ray.start.X,ray.start.Y,ray.start.Z, ray.end.X,ray.end.Y,ray.end.Z ) );
  }

  return hit;
}

FHitResult APlayerControl::TraceAgainst( AActor* actor, const FVector2D& ScreenPosition )
{
  FHitResult hit;
  Ray ray;
  // Get the origin & direction of a ray that corresponds with ScreenPosition according to the sceneview.
  if( !UGameplayStatics::DeprojectScreenToWorld( this, ScreenPosition, ray.start, ray.dir ) )
  {
    error( FS( "Could not DeprojectScreenToWorld(%f %f)", ScreenPosition.X, ScreenPosition.Y ) );
  }
  ray.SetLen( 1e6f );
  
  return TraceAgainst( actor, ray );
}

FHitResult APlayerControl::TraceAgainst( AActor* actor, const Ray& ray )
{
  FHitResult hit;
  FCollisionQueryParams fcqp( true );
  if( !actor->ActorLineTraceSingle( hit, ray.start, ray.end, CollisionChannels[ "RayCast" ], fcqp ) )
  {
    warning( FS( "ActorLineTraceSingle (%s) failed %f %f %f => %f %f %f",
      *actor->GetName(), ray.start.X,ray.start.Y,ray.start.Z, ray.end.X,ray.end.Y,ray.end.Z ) );
  }

  if( hit.GetActor() )
  {
    //info( FS( "ActorLineTraceSingle (%s) hit `[%s/%s]` @ (%f %f %f)",
    //  *actor->GetName(), *hit.GetActor()->GetName(), *hit.GetComponent()->GetName(), hit.ImpactPoint.X, hit.ImpactPoint.Y, hit.ImpactPoint.Z ) );
  }
  else
  {
    info( "TraceAgainst: Nothing was hit" );
  }

  return hit;
}

FHitResult APlayerControl::RayPickSingle( const FVector2D& ScreenPosition )
{
  TArray<FHitResult> hits;
  Ray ray;
	if( !UGameplayStatics::DeprojectScreenToWorld( this, ScreenPosition, ray.start, ray.dir ) )
  {
    error( FS( "Could not DeprojectScreenToWorld(%f %f)", ScreenPosition.X, ScreenPosition.Y ) );
  }
  ray.SetLen( 1e6f );
  return RayPickSingle( ray );
}

FHitResult APlayerControl::RayPickSingle( const Ray& ray )
{
  TArray<FHitResult> hits;
  FCollisionQueryParams fcqp( true );
  // Returns closest blocking hit, checking all objects
  if( !GetWorld()->LineTraceMultiByProfile( hits, ray.start, ray.end, "RayCast", fcqp ) )
  {
    error( FS( "LineTraceMultiByProfile(RayCast) didn't hit anything, %f %f %f => %f %f %f",
      ray.start.X,ray.start.Y,ray.start.Z, ray.end.X,ray.end.Y,ray.end.Z ) );
  }

  // You get the first result.
  FHitResult result;
  if( hits.Num() )
    result = hits[0];
  return result;
}

vector<AGameObject*> APlayerControl::RayPickMulti( const FVector2D& ScreenPosition )
{
  TArray<FHitResult> hits;
  Ray ray;
	if( !UGameplayStatics::DeprojectScreenToWorld( this, ScreenPosition, ray.start, ray.dir ) )
	{
    error( FS( "Could not DeprojectScreenToWorld (%f %f)", ScreenPosition.X, ScreenPosition.Y ) );
	}
  ray.SetLen( 1e6f );
  return RayPickMulti( ray );
}

vector<AGameObject*> APlayerControl::RayPickMulti( const Ray& ray )
{
  // Casts a ray into the scene
  FCollisionQueryParams fcqp( true );
  TArray<FHitResult> hits;
  if( !GetWorld()->LineTraceMultiByProfile( hits, ray.start, ray.end, "RayCast", fcqp ) )
  {
    error( FS( "LineTraceMultiByProfile(RayCast) didn't hit anything, %f %f %f => %f %f %f",
      ray.start.X,ray.start.Y,ray.start.Z, ray.end.X,ray.end.Y,ray.end.Z ) );
  }

  vector<AGameObject*> objects;
  for( int i = 0; i < hits.Num(); i++ )
  {
    FVector v = hits[i].ImpactPoint;
    info( FS( "  - Hit result [%s/%s] @ %f %f %f", 
      *hits[i].GetActor()->GetName(), *hits[i].GetComponent()->GetName(), v.X, v.Y, v.Z ) );
    if( AGameObject *go = Cast< AGameObject >( hits[i].GetActor() ) )
      objects += go;
  }
  return objects;
}

vector<AGameObject*> APlayerControl::FrustumPick( const FBox2DU& box )
{
  return FrustumPick( box, {}, {} );
}

// If InTypes is EMPTY, then it picks any type
vector<AGameObject*> APlayerControl::FrustumPick( const FBox2DU& box, 
  set< TSubclassOf<AGameObject> > AcceptedTypes,
  set< TSubclassOf<AGameObject> > NotTypes )
{
  // A 0 area box picks with a ray from TL corner of the click.
  if( !box.GetArea() ) {
    // use a ray pick
    //info( FS( "Point %f %f using a ray", box.TL().X, box.TL().Y ) );
    return RayPickMulti( box.TL() );
  }

  vector<FVector2D> pts = { box.TL(), box.BL(), box.BR(), box.TR() };
  vector<Ray> rays;

  FVector centerP(0,0,0);
  // get the 4 rays of the frustum
  for( int i = 0; i < pts.size(); i++ )
  {
    Ray ray;
	  if( !UGameplayStatics::DeprojectScreenToWorld( this, pts[i], ray.start, ray.dir ) )
	  {
      error( FS( "Could not DeprojectScreenToWorld to world for %f %f", pts[i].X, pts[i].Y ) );
    }

    ray.SetLen( 1e4f );  // WATCH THIS NUMBER IS NOT TOO LARGE, 1e6f causes imprecision
    rays.push_back( ray );
    centerP += ray.start;
  }
  centerP /= rays.size();
  
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
  map< float, vector<AGameObject*> > objects;
  for( Team* team : Game->gm->teams )
  {
    for( AGameObject* go : team->units )
    {
      // You cannot select dead objects
      if( go->IsPendingKill()   ||   go->Dead )  skip;

      // If the accepted types collection was specified, and the type is not in accepted types, skip
      if( AcceptedTypes.size() && ( !go->IsAny( AcceptedTypes ) ) )  skip;

      // If you are in the not types group, skip also
      if( go->IsAny( NotTypes ) ) skip;

      FBox box = go->hitBounds->Bounds.GetBox();
      
      // Selection by mesh's bounding box is best.
      if( selectionVolume.IntersectBox( box.GetCenter(), box.GetExtent() ) )
      {
        float d = FVector::Dist( centerP, go->Pos );
        info( FS( "%s is %f units away from camera (%f %f %f) => (%f %f %f)", 
          *go->Stats.Name, d, centerP.X,centerP.Y,centerP.Z, go->Pos.X,go->Pos.Y,go->Pos.Z ) );
        objects[ d ].push_back( go ); // Possibility of equidistant objects
      }
    }
  }

  // Form a linear vector from the map (equidistant objects adjacent)
  vector<AGameObject*> os;
  for( pair< const float, vector<AGameObject*> > p : objects )
    for( AGameObject* go : p.second )
      os += go;
  return os;
}

vector<AGameObject*> APlayerControl::ShapePick( FVector pos, FCollisionShape shape )
{
  return ShapePickExcept( pos, shape, {}, {} );
} 

vector<AGameObject*> APlayerControl::ShapePickExcept( FVector pos, FCollisionShape shape,
  set< TSubclassOf<AGameObject> > AcceptedTypes, set< TSubclassOf<AGameObject> > NotTypes )
{
  // We use the base shapepick, then filter
  FCollisionQueryParams fcqp;
  TArray<FOverlapResult> overlaps;
  FQuat quat( 0.f, 0.f, 0.f, 0.f );
  FCollisionObjectQueryParams objectTypes = FCollisionObjectQueryParams( 
    FCollisionObjectQueryParams::InitType::AllObjects );
  // To query using a specific object TYPE, use OverlapMultiByProfile()
  GetWorld()->OverlapMultiByObjectType( overlaps, pos, quat, objectTypes, shape, fcqp );
  
  // indicate where the search shape is.
  //DrawDebugCylinder( GetWorld(), pos, pos + FVector( 0, 0, shape.Capsule.HalfHeight*2.f ), shape.Capsule.Radius,
  //  16, FColor::Yellow, 1, 10.f, 0 );
  //info( FS( "Shape overlaps %d units", overlaps.Num() ) );

  map< float, vector<AGameObject*> > objects;
  for( int i = 0; i < overlaps.Num(); i++ )
  {
    if( AGameObject* go = Cast<AGameObject>( overlaps[i].GetActor() ) )
    {
      if( go->Dead ) skip;

      if( AcceptedTypes.size()   &&   !go->IsAny( AcceptedTypes ) )  skip;

      if( go->IsAny( NotTypes ) )  skip;
      
      // Duplicates DO happen
      if( !in( objects, go ) )
      {
        float d = FVector::Dist( pos, go->Pos ); // order 
        objects[ d ].push_back( go );
        //info( FS( "  * %s", *go->GetName() ) );
        //DrawDebugCylinder( GetWorld(), go->Pos, go->Pos + FVector(0,0,go->Height()),
        //  go->Radius(), 16, FColor::Green, 1, 10.f, 0 );
      }
    }
  }

  vector<AGameObject*> os;
  for( pair< const float, vector<AGameObject*> > p : objects )
    for( AGameObject* go : p.second )
      os += go;
  return os;
}

vector<AGameObject*> APlayerControl::ComponentPickExcept( AGameObject* object, UPrimitiveComponent* up, vector<AGameObject*> except,
  FString queryObjectType, vector<FString> intersectableTypes )
{
  FComponentQueryParams fqp;
  except += object;

  // this will cause the object to pick itself if you don't put it in the except group
  for( AGameObject* o : except )
    if( o )  fqp.AddIgnoredActor( o );

  FCollisionObjectQueryParams objectTypes = FCollisionObjectQueryParams( 
    FCollisionObjectQueryParams::InitType::AllObjects );

  for( FString channelName : intersectableTypes )
  {
    ECollisionChannel channel = CollisionChannels[channelName];
    objectTypes.AddObjectTypesToQuery( channel ); // Channel 3 is the RESOURCES channel.
  }

  TArray<FOverlapResult> overlaps;
  FRotator ro = up->GetComponentRotation();
  FVector ve = up->GetComponentLocation();

  // QUERY on channel 
  ECollisionChannel queryChannel = CollisionChannels[queryObjectType];
  GetWorld()->ComponentOverlapMultiByChannel( overlaps, up, ve, ro, queryChannel, fqp, objectTypes );
  
  map<float, AGameObject*> objects;
  for( int i = 0; i < overlaps.Num(); i++ )
  {
    if( AGameObject* go = Cast<AGameObject>( overlaps[i].GetActor() ) )
    {
      float d = FVector::Dist( object->Pos, go->Pos );
      objects[ d ] = go;
    }
  }

  return MakeVectorS( objects );
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


