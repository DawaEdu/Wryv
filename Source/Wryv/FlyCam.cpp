#include "Wryv.h"

#include "GameFramework/PlayerInput.h"

#include "AIProfile.h"
#include "Building.h"
#include "Enums.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "Goldmine.h"
#include "GroundPlane.h"
#include "Pathfinder.h"
#include "Peasant.h"
#include "PlayerControl.h"
#include "PlayerStartPosition.h"
#include "Projectile.h"
#include "Resource.h"
#include "Shape.h"
#include "Stone.h"
#include "TheHUD.h"
#include "Tree.h"
#include "Widget3D.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

FName AFlyCam::AttackTargetName = "AttackTarget";
FName AFlyCam::FollowTargetName = "FollowTarget";
FName AFlyCam::SelectedTargetName = "SelTarget";

// Sets default values
AFlyCam::AFlyCam( const FObjectInitializer& PCIP ) : APawn( PCIP )
{
  LOG( "AFlyCam::AFlyCam() ctor" );

  // Set this character to call Tick() every frame.
  // Turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = 1;
  PrimaryActorTick.bTickEvenWhenPaused = 1;
  ghost = 0;
  floor = 0;
  pathfinder = 0;
  setupLevel = 0;
  CheckerSphereRadius = 1.f;

  // Make the fog of war instance.
  MovementComponent = PCIP.CreateDefaultSubobject<UFloatingPawnMovement>( this, ADefaultPawn::MovementComponentName );
  MovementComponent->SetTickableWhenPaused( true );

  DummyRoot = PCIP.CreateDefaultSubobject<USceneComponent>( this, TEXT( "Dummy" ) );
  SetRootComponent( DummyRoot );
  MainCamera = PCIP.CreateDefaultSubobject<UCameraComponent>( this, TEXT( "MainCamera" ) );
  MainCamera->AttachTo( DummyRoot );
  OrthoCam = PCIP.CreateDefaultSubobject<UCameraComponent>( this, TEXT( "OrthoCam" ) );
  OrthoCam->AttachTo( DummyRoot );
  
  CameraMovementSpeed = 1000.f;
  VizGrid = 0;
  VizPassibles = 0;
  CheckerClass = 0;
  LineClass = 0;
  WaypointFlagClass = 0;
  BlockedColor = FLinearColor( .9,.2,.2,.5 ); //dark-reddish color
}

// Called when the game starts or when spawned
void AFlyCam::BeginPlay()
{
  LOG( "AFlyCam::BeginPlay()" );
	Super::BeginPlay();

  // Keep checkersphere bonus to below 1.f
  CheckerSphereRadius = FMath::Clamp( CheckerSphereRadius, 0.25f, 1.f );

  GroundMarker = Game->Make<AShape>( GroundMarkerClass );
}

void AFlyCam::SetupPlayerInputComponent( UInputComponent* InputComponent )
{
  LOG( "AFlyCam::SetupPlayerInputComponent()" );
  check( InputComponent );
  Super::SetupPlayerInputComponent( InputComponent );
  this->InputComponent = InputComponent;
  this->InputComponent->SetTickableWhenPaused(true);

  //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("CameraUp", EKeys::PageUp, 1.f));
  //UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("CameraDown", EKeys::PageDown, 1.f));
  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::W, 1.f));
  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::S, -1.f));
  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::Up, 1.f));
  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveForward", EKeys::Down, -1.f));
  
  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveRight", EKeys::D, 1.f));
  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveRight", EKeys::A, -1.f));
  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveRight", EKeys::Right, 1.f));
  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MoveRight", EKeys::Left, -1.f));

  UPlayerInput::AddEngineDefinedAxisMapping(FInputAxisKeyMapping("MouseClickedLMB", EKeys::Q, -1.f));
  
  // The "string" (eg. "Forward") is the mapping as defined in Project Settings / Input / Action Mappings.
  // We just associate the "Forward" action (which is bound to a key in 
  InputComponent->BindAxis( "Forward", this, &AFlyCam::MoveForward );
  InputComponent->BindAxis( "Left", this, &AFlyCam::MoveLeft );
  InputComponent->BindAxis( "Back", this, &AFlyCam::MoveBack );
  InputComponent->BindAxis( "Right", this, &AFlyCam::MoveRight );
  InputComponent->BindAxis( "MouseX", this, &AFlyCam::MouseMovedX );
  InputComponent->BindAxis( "MouseY", this, &AFlyCam::MouseMovedY );

  //InputComponent->AddAxisMapping(FInputAxisKeyMapping("CameraUp", EKeys::PageUp, 1.f));
  //InputComponent->AddAxisMapping(FInputAxisKeyMapping("CameraDown", EKeys::PageDown, 1.f));
  InputComponent->BindAxis( "CameraUp", this, &AFlyCam::MoveCameraZUp );
  InputComponent->BindAxis( "CameraDown", this, &AFlyCam::MoveCameraZDown );
  InputComponent->BindAxis( "CameraPitchUp", this, &AFlyCam::MoveCameraPitchUp );
  InputComponent->BindAxis( "CameraPitchDown", this, &AFlyCam::MoveCameraPitchDown );
  InputComponent->BindAxis( "CameraRotateCW", this, &AFlyCam::MoveCameraRotateCW );
  InputComponent->BindAxis( "CameraRotateCCW", this, &AFlyCam::MoveCameraRotateCCW );
  
  InputComponent->BindAction( "MouseClickedLMB", IE_Pressed, this, &AFlyCam::MouseDownLeft );
  InputComponent->BindAction( "MouseClickedLMB", IE_Released, this, &AFlyCam::MouseUpLeft );
  InputComponent->BindAction( "MouseClickedRMB", IE_Pressed, this, &AFlyCam::MouseDownRight );
  InputComponent->BindAction( "MouseClickedRMB", IE_Released, this, &AFlyCam::MouseUpRight );

  // Bind a key press to displaying the menu
  //InputComponent->BindKey( EKeys::PageUp, IE_Pressed, this, &AFlyCam::MoveCameraZUp );
  //InputComponent->BindKey( EKeys::PageDown, IE_Pressed, this, &AFlyCam::MoveCameraZDown );
  
  // Start the background music. Since we don't want attenuation,
  // we play the sound attached to the RootComponent of the Camera object,
  // and supply no further arguments.
  //music = UGameplayStatics::SpawnSoundAttached( bkgMusic, RootComponent );
  sfxVolume = 1.f;
}

void AFlyCam::MarkAsSelected( AGameObject* object )
{
  if( object->HasChildWithTag( SelectedTargetName ) ) {
    LOG( "%s already has a %s", *object->Stats.Name, *SelectedTargetName.ToString() );
    return;
  }

  AWidget3D* widget = Game->Make<AWidget3D>( SelectorClass, object->team );
  if( !widget ) { error( "Widget3d couldn't be created" ); return; }
  
  widget->Tags.Add( SelectedTargetName );
  float r = object->HitBoundsCylindricalRadius() * 1.5f;
  widget->SetActorScale3D( FVector(r,r,r) );
  widget->SetMaterialColors( "Color", FLinearColor(0,1,0,1) );
  object->AddChild( widget );
}

void AFlyCam::MarkAsFollow( AGameObject* object )
{
  // only select as follow target if its not already an attack target of something
  // ( attack priorities over follow )
  if( object->HasChildWithTag( FollowTargetName ) ) {
    //LOG( "%s already marked as follow", *object->Name );
    return; // already marked as an attack target
  }

  AWidget3D* widget = Game->Make<AWidget3D>( SelectorClass, object->team );
  if( !widget ) { error( "Widget3d couldn't be created" ); return; }
  
  widget->Tags.Add( FollowTargetName );
  float r = object->HitBoundsCylindricalRadius() * 1.5f;
  widget->SetActorScale3D( FVector(r,r,r) );
  widget->SetMaterialColors( "Color", FLinearColor(1,1,0,1) );
  object->AddChild( widget );
}

void AFlyCam::MarkAsAttack( AGameObject* object )
{
  // Check that it doesn't already have a Selector-typed child
  if( object->HasChildWithTag( AttackTargetName ) ) {
    //LOG( "%s already marked as attack", *object->Name );
    return; // already marked as an attack target
  }

  AWidget3D* widget = Game->Make<AWidget3D>( SelectorClass, object->team );
  if( !widget ) { error( "Widget3d couldn't be created" ); return; }
  
  widget->Tags.Add( AttackTargetName );
  float r = object->HitBoundsCylindricalRadius() * 1.5f;
  widget->SetActorScale3D( FVector( r,r,r ) );
  widget->SetMaterialColors( "Color", FLinearColor(1,0,0,1) );
  object->AddChild( widget );
}


void AFlyCam::ClearGhost()
{
  // Remove the ghost
  if( ghost )
  {
    ghost->Cleanup();
    ghost = 0;
  }
  else
  {
    error( "Tried to clean up ghost when no ghost was set" );
  }

  //for( ABuilding* gho : ghosts )
  //{
  //  gho->Cleanup();
  //}
  //ghosts.clear();
}

void AFlyCam::LoadLevel( FName levelName )
{
  LOG( "AFlyCam::LoadLevel(%s)", *levelName.ToString() );
  Game->gm->state = Running;

  // synchrononously loads level
  Game->pc->Pause(); // pause so no actors tick.

  // unload level out from under the actors.
  LOG( "Loading level %s", *levelName.GetPlainNameString() );
  UnloadLevel() ; // unload the current level and dispose of all info in it
  UGameplayStatics::OpenLevel( GetWorld(), levelName );
  OnLevelLoaded();

  // unpause so actors tick again.. 
  Game->pc->SetPause( false ); // pause so no actors tick.
}

void AFlyCam::OnLevelLoaded()
{
  LOG( "AFlyCam::OnLevelLoaded()" );
  FindFloor();
  RetrievePointers();
  InitializePathfinding();
  
  setupLevel = 1;
}

void AFlyCam::SetObjectsOnGround()
{
  TTransArray<AActor*> actors = GetWorld()->GetLevel(0)->Actors;

  // Set all actors on ground.
  for( int i = 0; i < actors.Num(); i++ )
  {
    AGameObject* go = Cast<AGameObject>( actors[i] );
    if( !go ) skip;

    if( Cast<AGroundPlane>( go ) )
    {
      info( "NOT PUTTING THE GROUND PLANE ON THE FLOOR.." );
      skip;
    }

    FVector pos = go->Pos;
    //info( FS( "Object %s started @ %f %f %f", *go->GetName(), pos.X, pos.Y, pos.Z ) );
    if( go->Grounds   &&   SetOnGround( pos ) )
    {
      // Pos & Dest must be set on the ground
      go->SetPosition( pos );
      //info( FS( "Object %s moved down to @ %f %f %f", *go->GetName(), pos.X, pos.Y, pos.Z ) );
    }
    else
    {
      // The resource can't hit the ground
      error( FS( "Object %s cannot hit the ground plane", *go->Stats.Name ) );
    }
  }
}

void AFlyCam::InitializePathfinding()
{
  if( !floor ) {
    error( "NO FLOOR, cannot initialize pathfinding" );
    return;
  }
  
  LOG( "Initializing pathfinding" );
  
  if( pathfinder ) {
    warning( "Pathfinder already initialized" );
    return;
  }
  SetObjectsOnGround();

  // (or a floorplane can be constructed as a failsafe).
  // Initialize a bunch of bounding spheres
  if( !Rows )  Rows = Cols = 20;
  if( pathfinder )  delete pathfinder;
  pathfinder = new Pathfinder( Rows, Cols );

  // get the actual spacing between nodes, then divide by 2
  // use 95% prox to ensure that
  FBox floorBox = floor->GetBox();
  FBox reducedBox = floor->GetReducedBox();

  // Radius of spheres is based on reduced box size.
  FVector diameterScale = reducedBox.GetSize() / FVector( Rows, Cols, 1.f );
  diameterScale *= CheckerSphereRadius; //CheckerSphereRadius is typically 1.f.. if you make it smaller,
  // then the test-spheres get smaller, making the pathfinder more capable of finding cracks.
  diameterScale.Z = diameterScale.X; // Keep it rounder

  // adjacent spheres don't overlap
  for( int row = 0; row < Rows; row++ )
  {
    for( int col = 0; col < Cols; col++ )
    {
      // Lay out the spheres
      FVector fraction( (row + .5f)/Rows, (col + .5f)/Cols, 1.f );

      // For the landscape, the lower left is actually box.Min + ext.XY0.
      FVector p = FMath::Lerp( reducedBox.Min, reducedBox.Max, fraction );
      p.Z = reducedBox.Max.Z + 1.f;
      Coord coord( row, col );
      int idx = coord.index();
      pathfinder->nodes[ idx ]->index = idx;
      
      if( !SetOnGround( p ) )
      {
        // The point P is not on the ground, so it's outside the floorbox.
        pathfinder->nodes[ idx ]->terrain = Terrain::Impassible;
        pathfinder->nodes[ idx ]->point = p;
      }
      else
      {
        pathfinder->nodes[ idx ]->point = p;
        AShape* sphere = Game->Make<AShape>( CheckerClass, Game->gm->neutralTeam, p );
        if( !sphere )
        {
          error( "Couldn't make intersection sphere" );
          skip;
        }
        sphere->SetSize( diameterScale );
        
        // Pick objects intersecting with the sphere. If anything intersects, then
        // the node is regarded as impassible.
        vector<AGameObject*> forbidden = { floor };

        // The hitBox is used & should be configured inside UE4 SETTINGS
        // collision profile to determine what types of objects to intersect with.
        // Mesh-Mesh intersection is kind of buggy and not very fast, so I avoided it here.
        vector<AGameObject*> intns = Game->pc->ComponentPickExcept( sphere, sphere->hitBox, forbidden,
          "Checkers", {"SolidObstacle"} );
        
        if( intns.size() ) {
          pathfinder->nodes[ idx ]->terrain = Terrain::Impassible;
          sphere->SetColor( FLinearColor::Red );
          //LOG( "Shape %s had collisions:", *sphere->GetName() );
          //for( AGameObject* g : intns ) {
          //  LOG( "  * %s", *g->GetName() ); }
        }
        else {
          pathfinder->nodes[ idx ]->terrain = Terrain::Passible;
          sphere->SetColor( FLinearColor::White );
          //LOG( "Shape %s had no collisions", *sphere->GetName() );
        }

        sphere->Cleanup(); // Don't show the sphere visualization
      }
      pathfinder->updateGraphConnections( coord );
    }
  }
  
  GetWorld()->ForceGarbageCollection(true); // Force a cleanup of all the Sphere actors

  if( !VizGrid )  return;

  // Visualization of graph
  set<Edge*> edges;
  for( int i = 0; i < pathfinder->nodes.size(); i++ )
  {
    GraphNode *node = pathfinder->nodes[i];
    if( node->terrain == Passible && VizPassibles )
    {
      AShape *vizSphere = Game->Make<AShape>( CheckerClass, Game->gm->neutralTeam, node->point );
      vizSphere->SetSize( diameterScale );
    }
    else if( node->terrain == Impassible )
    {
      AShape *vizSphere = Game->Make<AShape>( CheckerClass, Game->gm->neutralTeam, node->point );
      vizSphere->SetSize( diameterScale );
      vizSphere->SetColor( FLinearColor::Red );
    }

    // create a node and edge connections
    for( int j = 0; j < node->edges.size(); j++ )
      edges.insert( node->edges[j] );
  }

  for( Edge* e : edges )
  {
    // set size of edge proportional to distance between nodes
    MakeLine( e->src->point, e->dst->point, FLinearColor::White );
  }
}

void AFlyCam::UnloadLevel()
{
  //Game->pc->EndPlay( EEndPlayReason::LevelTransition );
  //GetWorld()->GetLevel(0)->Actors[0]->EndPlay( EEndPlayReason::Destroyed );
  Game->gm->EndPlay( EEndPlayReason::LevelTransition );
  
  // destroy all the actors
  for( int i = 0; i < GetWorld()->GetLevel(0)->Actors.Num(); i++ )
  {
    AActor *a = GetWorld()->GetLevel(0)->Actors[i];
    if( a )  a->EndPlay( EEndPlayReason::Destroyed );
  }

  //Game->pc->EndPlay( EEndPlayReason::LevelTransition );

  ghost = 0;
  floor = 0;
  delete pathfinder;
  pathfinder = 0;
  setupLevel = 0;

}

void AFlyCam::SetCameraPosition( FVector2D perc )
{
  if( !floor ) {
    LOG( "no floor" );
    return;
  }
  
  FVector P = GetActorLocation(); // restore the z value after movement
  FVector fwd = MainCamera->GetForwardVector();

  // Find coordinates of click on ground plane
  // X & Y are reversed because Y goes right, X goes fwd.
  // Extents are HALF extents

  // Normal object (with centered origin)
  FBox reducedBox = floor->GetReducedBox();
  FVector G = reducedBox.Min + reducedBox.GetExtent() * 2.f * FVector( 1.f-perc.Y, perc.X, 0 );
  
  //FVector ext = box.GetExtent();
  //ext.Z = 0; // no z-measure
  //FVector G = (box.Min + ext) + ext * FVector( 1.f-perc.Y, perc.X, 0 );

  // for the ground when its not a plane
  //G.Z = box.Max.Z;

  float len = FVector::Dist( P, G );

  FVector P2 = G - len * fwd;
  P2.Z = P.Z; // nail z-value to old z-value to prevent camera climb
  
  // Move the camera back from the point on the plane in -cameraDir
  // vertically move the camera up to the higher plane
  // Move BACK in the direction of the fwd vector length of previous location
  //pos += camera->GetForwardVector() * -startLoc.Size();
  
  FQuat q( 0.f, 0.f, 0.f, 0.f );
  //LOG( "New camera pos %f %f %f", pos.X, pos.Y, pos.Z );
  SetActorLocationAndRotation( P2, q );

  // Compute frustum corners collision with ground plane.
  // find the frustum corners
  //FConvexVolume frustum;
  //GetViewFrustumBounds( frustum, this->camera->MarkRenderTransformDirty
}

UMaterialInterface* AFlyCam::GetMaterial( FLinearColor color )
{
  UMaterialInstanceDynamic* material = 0;
  map<FLinearColor,UMaterialInstanceDynamic*>::iterator it = Colors.find( color );
  if( it == Colors.end() )
  {
    material = UMaterialInstanceDynamic::Create( BaseWhiteInterface, this );
    material->SetVectorParameterValue( FName( "Color" ), color );
    material->SetVectorParameterValue( FName( "None" ), color ); // the parameter gets called "None" for some reason
    Colors[ color ] = material;
    ColorMaterials.Push( material );
  }
  else
    material = it->second;

  return material;
}

void AFlyCam::Visualize( FVector& v, float s, FLinearColor color, float time )
{
  AShape* shape = Game->Make<AShape>( VizClass, Game->gm->neutralTeam, v );
  shape->SetSize( FVector( s ) );
  shape->SetColor( color );
  shape->MaxLifeTime = time;
  viz.push_back( shape );
}

void AFlyCam::Visualize( vector<FVector>& v, float s, FLinearColor startColor, FLinearColor endColor, float time )
{
  for( int i = 0; i < v.size(); i++ )
  {
    //LOG( "Pathway is (%f %f %f)", v[i].X, v[i].Y, v[i].Z );
    float p = (float)i / v.size();
    FLinearColor color = FLinearColor::LerpUsingHSV( startColor, endColor, p );
    Visualize( v[i], s, color, time );
  }
}

void AFlyCam::DrawDebug( Ray ray, float size, FLinearColor color, float time )
{
  DrawDebugLine( GetWorld(), ray.start, ray.end, color.ToFColor(0), 0, time, 0, size );
}

void AFlyCam::DrawDebug( FVector pt, float size, FLinearColor color, float time )
{
  DrawDebugPoint( GetWorld(), pt, size, color.ToFColor(0), 0, time, 0 );
}

void AFlyCam::DrawDebug( FVector start, FVector end, float thickness, FLinearColor color, float time )
{
  DrawDebugLine( GetWorld(), start, end, color.ToFColor(0), 0, time, 0, thickness );
}

void AFlyCam::ClearViz()
{
  for( int i = 0; i < viz.size(); i++ )
    viz[i]->Cleanup();
  viz.clear();
}

AGameObject* AFlyCam::MakeLine( FVector Start, FVector End, FLinearColor color )
{
  // The line is a unit line
  FVector dir = End - Start;
  float len = dir.Size();
  if( !len ) {
    error( FS( "Making an edge of 0 length from %f %f %f to %f %f %f",
      Start.X,Start.Y,Start.Z, End.X,End.Y,End.Z ) ) ;
    return 0;
  }
  dir /= len;

  AShape *line = Game->Make<AShape>( LineClass, Game->gm->neutralTeam, Start );
  line->SetSize( FVector(len) );
  line->SetColor( color );
  line->SetActorRotation( dir.Rotation() );
  return line;
}

void AFlyCam::RetrievePointers()
{
  Game->pc = Cast<APlayerControl>( GetWorld()->GetFirstPlayerController() );
  Game->hud = Cast<ATheHUD>( Game->pc->GetHUD() );
  Game->gm = Cast<AWryvGameMode>( GetWorld()->GetAuthGameMode() );
  Game->gs = Cast<AWryvGameState>( GetWorld()->GetGameState() );
  Game->flycam = this;
  //info( FS( "PC: %d, HUD %d, GM: %d, GS %d, flycam %d",
  //  Game->pc, Game->hud, Game->gm, Game->gs, Game->flycam ) );

  // Check that each is an instance of the class it should be.
  CheckInstance<UWryvGameInstance>( Game );
  CheckInstance<APlayerControl>( Game->pc );
  CheckInstance<ATheHUD>( Game->hud );
  CheckInstance<AWryvGameMode>( Game->gm );
  CheckInstance<AWryvGameState>( Game->gs );
  CheckInstance<AFlyCam>( Game->flycam );
}

void AFlyCam::debug( int slot, FColor color, FString mess )
{
	if( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( slot, 5.f, color, mess );
	}
}

FVector2D AFlyCam::getMousePos()
{
  FVector2D mouse(0,0);
  if( !Game->pc->GetMousePosition( mouse.X, mouse.Y ) )
  {
    // Mouse offscreen
  }
  return mouse;
}

bool AFlyCam::SetOnGround( FVector& v )
{
  if( !floor ) {
    error( "No floor" );
    return 0;
  }
  
  FVector v2 = v;
  v2.Z += floor->GetBox().GetSize().Z; /// Pick up far above the floor
  
  FHitResult hit = Game->pc->TraceAgainst( floor, Ray(v2, FVector( 0, 0, -1 ), 1e4f ) );
  if( hit.GetActor() )
  {
    v = hit.ImpactPoint;
    return 1;
  }
  else
  {
    // no hit, no change
    warning( FS( "Point %f %f %f cannot hit the ground", v.X, v.Y, v.Z ) );
    Visualize( v, 50.f, FLinearColor::Blue, 10.f );
    return 0;
  }
}

void AFlyCam::FindFloor()
{
  floor = 0;
  ULevel* level = GetWorld()->GetLevel(0);
  TTransArray<AActor*>& actors = level->Actors;

  // Here, the floor wasn't found above, so search by name.
  for( int i = 0; i < actors.Num(); i++ )
  {
    if( AGroundPlane* gp = Cast<AGroundPlane>( actors[i] ) )
      floor = gp;

    if( APlayerStartPosition* psp = Cast<APlayerStartPosition>( actors[i] ) )
    {
      info( FS( "Found PlayerStartPosition %d", psp->TeamNumber ) );
      if( psp->TeamNumber < Game->gm->teams.size() )
      {
        Team* team = Game->gm->teams[ psp->TeamNumber ];
        team->ai = psp->aiProfile->ai;
        info( FS( "Player %d has AI `%s`", psp->TeamNumber, *team->ai.ToString() ) );
      }
    }
  }

  // Form the world bounds. You cannot use 
  // FBox box = ALevelBounds::CalculateLevelBounds( level );
  // because it will be too huge if you're using a skydome
  FBox box(0);
  for( int i = 0; i < actors.Num(); i++ )
    if( AGameObject* go = Cast<AGameObject>( actors[i] ) )
      box += go->GetComponentsBoundingBox();

  if( !floor )
  {
    error( "Floor not found" );
    // Create a cubic floor underneath level bounds.
    //FBox box = level->LevelBoundsActor->GetComponentsBoundingBox(); X
    //FBox box = ALevelBounds::CalculateLevelBounds( level ); X
    Print( "level bounds", box );
    FVector size = box.GetSize();
    AShape* shape = Game->Make<AShape>( FloorPlaneBackupClass, Game->gm->neutralTeam );
    shape->SetSize( FVector( size.X, size.Y, 100.f ) );
    shape->SetPosition( FVector( 0, 0, box.Min.Z - box.GetExtent().Z - 100.f ) );
    shape->SetColor( FLinearColor::Yellow );
  }

  // create the fog of war now
  ////fogOfWar = Game->Make<AFogOfWar>( FOGOFWAR, Game->gm->neutralTeam );
  ////fogOfWar->Init( floor->GetBox() );
}

void AFlyCam::MouseUpLeft()
{
  // Passes thru to HUD
  Game->hud->MouseUpLeft( getMousePos() );
}

void AFlyCam::MouseDownLeft()
{
  // Covers all click behavior.
  Game->hud->MouseDownLeft( getMousePos() );
}

void AFlyCam::MouseUpRight()
{
  
}

vector<FVector> AFlyCam::GenerateGroundPositions( FVector P, int numGridPos )
{
  vector<FVector> positions;

  FVector avgPos = Zero;
  for( AGameObject* go : Game->hud->Selected )
    avgPos += go->Pos;
  avgPos /= Game->hud->Selected.size();

  FVector travelDir = P - avgPos;
  if( float len = travelDir.Size() )
    travelDir /= len;
  else
  {
    warning( "Travel length 0" );
    return positions;
  }

  // The side vector is going to be a result of crossing with the up vector
  FVector right = FVector::CrossProduct( UnitZ, travelDir );
  float largestRadius = Game->hud->Selected.front()->repulsionBounds->GetScaledSphereRadius();
  for( int i = 0; i < Game->hud->Selected.size(); i++ )
  {
    float r = Game->hud->Selected[i]->repulsionBounds->GetScaledSphereRadius();
    if( r > largestRadius )
      largestRadius = Game->hud->Selected[i]->repulsionBounds->GetScaledSphereRadius();
  }
  largestRadius *= 2.f; // diameter spacing, x4
  
  // form the grid centered around the center point
  for( int i = 0; i < Game->hud->Selected.size(); i++ )
  {
    int x = i % numGridPos;
    int y = i / numGridPos;
    float xP = largestRadius * (x - (numGridPos-1)/2.f);
    float yP = largestRadius * (y - (numGridPos-1)/2.f);
    FVector pos = P   +   right*xP - travelDir*yP; // use - travel dir so 1st point in front row
    
    positions.push_back( pos );
  }
  return positions;
}

// Targets selected units @ another unit or groundpos
// (queued NextAction/left click or right click behavior).
void AFlyCam::Target()
{
  FHitResult hit = Game->pc->RayPickSingle( getMousePos(),
    MakeSet( Game->hud->Selectables ), MakeSet( Game->hud->Unselectables ) );
  AGameObject* target = Cast<AGameObject>( hit.GetActor() );
  if( !target )
  {
    info( "Right clicked on nothing" );
    return;
  }

  FVector P = hit.ImpactPoint;
  SetOnGround( P );
  //DrawDebug( P, 32.f, FLinearColor::Yellow, 10.f );
  // Hit the floor target, which means send units to ground position
  if( target == floor )
  {
    // Arrange the units in some formation around hit.ImpactPoint.
    // Units targeted in group cannot have the same destination.
    // The group of units will be travelling in an average direction
    GroundMarker->SetPosition( P );
    GroundMarker->SetDestination( P - 100.f*UnitZ );
    
    int numGridPos = FMath::CeilToInt( sqrtf( Game->hud->Selected.size() ) );
    vector<FVector> positions = GenerateGroundPositions( P, numGridPos );
    for( int i = 0; i < Game->hud->Selected.size(); i++ )
    {
      AGameObject* go = Game->hud->Selected[i];
      //go->GoToGroundPosition( go->Pos + offset ); // C++ Code Command
      if( Game->pc->IsAnyKeyDown( { EKeys::LeftShift, EKeys::RightShift } ) )
      {
        // When shift is down, we have to add the command to the list of commands for this unit.
        Game->EnqueueCommand( Command( Command::GoToGroundPosition, go->ID, positions[i] ) ); // Network command
      }
      else
      {
        // When shift is NOT down, we have to clear the unit's command set.
        Game->SetCommand( Command( Command::GoToGroundPosition, go->ID, positions[i] ) ); // Network command
      }
    }
  }
  else // Some object was right-clicked.
  {
    // An actor was hit by the click. Detect if friendly or not, and
    // launch a targetting cmd to it
    for( AGameObject* go : Game->hud->Selected )
    {
      if( Game->pc->IsAnyKeyDown( { EKeys::LeftShift, EKeys::RightShift } ) )
        Game->EnqueueCommand( Command( Command::Target, go->ID, target->ID ) );
      else
        Game->SetCommand( Command( Command::Target, go->ID, target->ID ) );
    }
  }
}

void AFlyCam::MouseDownRight()
{
  if( !Game->hud->Selected.size() )
  {
    info( "Nothing to command" );
    return;
  }

  if( ghost )
  {
    info( FS( "The building %s was cancelled", *ghost->Stats.Name ) );
    ClearGhost();
    return;
  }

  Target();
}

void AFlyCam::MouseMoved()
{
  RetrievePointers();
  FVector2D mouse = getMousePos();

  if( !setupLevel )
  {
    LOG( "Loading level" );
    OnLevelLoaded(); // This is here because it runs first for some reason (before ::Tick())
  }

  HotSpot* hitElt = Game->hud->MouseMoved( mouse );

  // if the mouse button is down, then its a drag event, else its a hover event
  if( Game->pc->IsKeyDown( EKeys::LeftMouseButton ) )
  {
    // 3D drag event, would be used for multiple object placement, or brushes.
  }
  else
  {
    // hover event. move the building ghost around etc.
    FHitResult hit = Game->pc->TraceAgainst( floor->Mesh, mouse );

    // If you're sliding the mouse along the floor, move the building along with if its set
    if( ghost )
    {
      if( hit.GetActor() == floor )
      {
        ghost->Pos = hit.ImpactPoint;
        if( !ghost->CanBePlaced() )
        {
          // Red, to indicate building cannot be placed here.
          ghost->SetMaterialColors( "Multiplier", BlockedColor );
        }
        else
        {
          // White to indicate placement success
          ghost->SetMaterialColors( "Multiplier", FLinearColor::White );
        }
      }
    }
  }
}

void AFlyCam::MouseMovedX( float amount )
{
  // Just calls MouseMoved() where mouse pixel
  // coords are extracted from getMousePos()
  MouseMoved();
}

void AFlyCam::MouseMovedY( float amount )
{
  MouseMoved();
}

void AFlyCam::MoveCameraZUp( float amount )
{
  if( Controller && amount )
  {
    FVector up( 0, 0, 1 );
    AddMovementInput( up, CameraMovementSpeed*amount );
  }
}

void AFlyCam::MoveCameraZDown( float amount )
{
  if( Controller && amount )
  {
    FVector down( 0, 0, -1 );
    AddMovementInput( down, CameraMovementSpeed*amount );
  }
}

void AFlyCam::MoveCameraPitchUp( float amount )
{
  if( Controller && amount )
  {
    //Game->pc->AddPitchInput( -1.f );
    AddControllerPitchInput( amount );
    //AddControllerPitchInput( amount );
    //MainCamera->AddRelativeRotation( FQuat( FVector(0,1,0), 0.5f ) );
    //MainCamera->RelativeRotation.Add( -1.f, 0.f, 0.f );
  }
}

void AFlyCam::MoveCameraPitchDown( float amount )
{
  if( Controller && amount )
  {
    //AddControllerPitchInput( amount );
    AddControllerPitchInput( -amount );
    //Game->pc->AddPitchInput( 1.f );
    //MainCamera->RelativeRotation.Add( 1.f, 0.f, 0.f );
    //LOG( "Camera rotation %f %f %f", MainCamera->RelativeRotation.Pitch, MainCamera->RelativeRotation.Yaw,
    //  MainCamera->RelativeRotation.Roll );
    //MainCamera->AddRelativeRotation( FQuat( FVector(0,1,0), 0.5f ) );
  }
}

void AFlyCam::MoveCameraRotateCW( float amount )
{
  if( Controller && amount )
  {
    AddControllerYawInput( amount );
  }
}

void AFlyCam::MoveCameraRotateCCW( float amount )
{
  if( Controller && amount )
  {
    AddControllerYawInput( -amount );
  }
}

void AFlyCam::MoveForward( float amount )
{
  // Gets called EACH FRAME (even if there's no input)
  if( Controller && amount )
  {
    FVector fwd = Game->pc->GetActorForwardVector(); //MainCamera->GetForwardVector();
    fwd.Z = 0.f;
    if( !fwd.SizeSquared() )  fwd.Y = -1.f;
    fwd.Normalize();
    AddMovementInput( fwd, CameraMovementSpeed*amount );
  }
}

void AFlyCam::MoveBack( float amount )
{
  if( Controller && amount )
  {
    FVector back = -Game->pc->GetActorForwardVector(); //-MainCamera->GetForwardVector();
    back.Z = 0.f;
    if( !back.SizeSquared() )  back.Y = 1.f;
    back.Normalize();
    AddMovementInput( back, CameraMovementSpeed*amount );
  }
}

void AFlyCam::MoveLeft( float amount )
{
  if( Controller && amount )
  {
    FVector left = -Game->pc->GetActorRightVector(); //-MainCamera->GetRightVector();
    left.Z = 0.f;
    if( !left.SizeSquared() )  left.X = -1.f;
    left.Normalize();
    AddMovementInput( left, CameraMovementSpeed*amount );
  }
}

void AFlyCam::MoveRight( float amount )
{
  if( Controller && amount )
  {
    FVector right = Game->pc->GetActorRightVector(); //MainCamera->GetRightVector();
    right.Z = 0.f;
    if( !right.SizeSquared() )  right.X = -1.f;
    right.Normalize();
    AddMovementInput( right, CameraMovementSpeed*amount );
  }
}

// Called every frame
void AFlyCam::Tick( float t )
{
	Super::Tick( t );
  if( !floor )
  {
    /// All levels must have a floor
    fatal( "ERROR: FLOOR NOT VALID" );
  }
}


