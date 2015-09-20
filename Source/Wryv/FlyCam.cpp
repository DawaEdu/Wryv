#include "Wryv.h"
#include "Building.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "GroundPlane.h"
#include "Pathfinder.h"
#include "Peasant.h"
#include "PlayerControl.h"
#include "Projectile.h"
#include "Resource.h"
#include "Shape.h"
#include "TheHUD.h"
#include "Types.h"
#include "UISounds.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "GameFramework/PlayerInput.h"

// Sets default values
AFlyCam::AFlyCam( const FObjectInitializer& PCIP ) : APawn( PCIP )
{
  LOG( "AFlyCam::AFlyCam() ctor" );

  // Set this character to call Tick() every frame.
  // Turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.bTickEvenWhenPaused = 1;
  ghost = 0;
  floor = 0;
  pathfinder = 0;
  setupLevel = 0;
  CheckerSphereRadius = 1.f;

  // Make the fog of war instance.
  MovementComponent = PCIP.CreateDefaultSubobject<UFloatingPawnMovement>( this, ADefaultPawn::MovementComponentName );
  MovementComponent->SetTickableWhenPaused( true );

  DummyRoot = PCIP.CreateDefaultSubobject<USceneComponent>( this, "Dummy" );
  SetRootComponent( DummyRoot );
  MainCamera = PCIP.CreateDefaultSubobject<UCameraComponent>( this, "MainCamera" );
  MainCamera->AttachTo( DummyRoot );
  OrthoCam = PCIP.CreateDefaultSubobject<UCameraComponent>( this, "OrthoCam" );
  OrthoCam->AttachTo( DummyRoot );
  
  CameraMovementSpeed = 1000.f;
  VizGrid = 0;
  VizPassibles = 0;
}

// Called when the game starts or when spawned
void AFlyCam::BeginPlay()
{
  LOG( "AFlyCam::BeginPlay()" );
	Super::BeginPlay();

  // Keep checkersphere bonus to below 1.f
  CheckerSphereRadius = FMath::Clamp( CheckerSphereRadius, 0.25f, 1.f );
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
  if( !floor ) {
    LOG( "No floor!" );
    return; // means there's no floor! level cannot be played
  }
  
  RetrievePointers();
  InitializePathfinding();

  float IMAX = 1000.f;
  for( int i = 0; i <= IMAX; i += 100 )
  {
    FVector v( 1000, 1000, i );
    FLinearColor color = FLinearColor::LerpUsingHSV( FLinearColor::Black, FLinearColor::White, i/IMAX );
    AShape* s = Game->Make<AShape>( Types::SHAPESPHERE, Game->gm->neutralTeam, v );
    s->SetColor( color );
    s->text = FS( "%d", i );
  }

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
    if( SetOnGround( pos ) )
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

  set<Types> intTypes = { Types::RESLUMBER, Types::RESSTONE, Types::RESGOLD };
  
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
        AShape* sphere = Game->Make<AShape>( SHAPESPHERE, Game->gm->neutralTeam, p );
        if( !sphere )
        {
          error( "Couldn't make intersection sphere" );
          skip;
        }
        sphere->SetSize( diameterScale );
        
        // Pick objects intersecting with the sphere. If anything intersects, then
        // the node is regarded as impassible.
        vector<AGameObject*> forbidden = { floor };

        // The hitBounds is used & should be configured inside UE4 SETTINGS
        // collision profile to determine what types of objects to intersect with.
        // Mesh-Mesh intersection is kind of buggy and not very fast, so I avoided it here.
        vector<AGameObject*> intns = Game->pc->ComponentPickExcept( sphere, sphere->hitBounds, forbidden,
          "Checkers", {"SolidObstacle"} );
        
        if( intns.size() ) {
          pathfinder->nodes[ idx ]->terrain = Terrain::Impassible;
          sphere->SetColor( FLinearColor::Red );
          LOG( "Shape %s had collisions:", *sphere->GetName() );
          for( AGameObject* g : intns )
          {
            LOG( "  * %s", *g->GetName() );
          }
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
    if( node->terrain == Passible )
    {
      if( VizPassibles )
      {
        AShape *vizSphere = Game->Make<AShape>( SHAPESPHERE, Game->gm->neutralTeam, node->point );
        vizSphere->SetSize( diameterScale );
      }
    }
    else
    {
      AShape *vizSphere = Game->Make<AShape>( SHAPESPHERE, Game->gm->neutralTeam, node->point );
      vizSphere->SetColor( FLinearColor::Red );
      vizSphere->SetSize( diameterScale );
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

void AFlyCam::Visualize( Types type, FVector& v, float s, FLinearColor color )
{
  AGameObject* go = Game->Make<AGameObject>( type, Game->gm->neutralTeam, v );
  go->SetSize( FVector(s) );
  go->SetColor( color );
  viz.push_back( go );
}

void AFlyCam::Visualize( Types type, vector<FVector>& v, float s, FLinearColor startColor, FLinearColor endColor )
{
  for( int i = 0; i < v.size(); i++ )
  {
    //LOG( "Pathway is (%f %f %f)", v[i].X, v[i].Y, v[i].Z );
    float p = (float)i / v.size();
    FLinearColor color = FLinearColor::LerpUsingHSV( startColor, endColor, p );
    Visualize( type, v[i], s, color );
  }
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

  AShape *line = Game->Make<AShape>( Types::SHAPEEDGE, Game->gm->neutralTeam, Start );
  line->SetSize( FVector(len) );
  line->SetColor( color );
  line->SetActorRotation( dir.Rotation() );
  return line;
}

void AFlyCam::RetrievePointers()
{
  Game->pc = Cast<APlayerControl>( GetWorld()->GetFirstPlayerController() );
  HotSpot::hud = Game->hud = Cast<ATheHUD>( Game->pc->GetHUD() );
  Game->gm = (AWryvGameMode*)GetWorld()->GetAuthGameMode();
  Game->gs = (AWryvGameState*)GetWorld()->GetGameState();
  Game->flycam = this;

  //info( FS( "PC: %d, HUD %d, GM: %d, GS %d, flycam %d",
  //  Game->pc, Game->hud, Game->gm, Game->gs, Game->flycam ) );
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

  //ClearViz();
  // Get ray hit with ground
  //Visualize( Types::SHAPESPHERE, v, 10.f, FLinearColor::Green );
  //Print( "v:", v );
  
  FVector v2 = v;
  v2.Z += floor->GetBox().GetSize().Z; /// Pick up far above the floor
  //Print( "v2:", v2 );
  //Visualize( Types::SHAPESPHERE, v2, 10.f, FLinearColor::Blue );

  FHitResult hit = Game->pc->TraceAgainst( floor, Ray(v2, FVector( 0, 0, -1 ), 1e4f) );
  if( hit.GetActor() )
  {
    v = hit.ImpactPoint;
    //Print( "impact @:", v2 );
    //Visualize( Types::SHAPESPHERE, v, 10.f, FLinearColor::Red );
    return 1;
  }
  else
  {
    // no hit, no change
    warning( FS( "Point %f %f %f cannot hit the ground", v.X, v.Y, v.Z ) );
    Visualize( Types::SHAPESPHERE, v, 50.f, FLinearColor::Blue );
    return 0;
  }
}

void AFlyCam::FindFloor()
{
  floor = 0;
  ULevel* level = GetWorld()->GetLevel(0);
  TTransArray<AActor*>& actors = level->Actors;

  // Here, the floor wasn't found above, so search by name.
  for( int i = 0; i < actors.Num() && !floor; i++ )
    if( AGroundPlane* gp = Cast<AGroundPlane>( actors[i] ) )
      floor = gp;

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
    
    //{
    //  AShape* shape = Game->Make<AShape>( SHAPECUBE, Game->gm->neutralTeam );
    //  shape->SetSize( FVector( size.X, size.Y, 100.f ) );
    //  shape->SetColor( FLinearColor::Blue );
    //  //shape->SetPosition( FVector( 0, 0, box.Min.Z - box.GetExtent().Z - 100.f ) );
    //  shape->SetPosition( FVector( 0, 0, box.Min.Z - box.GetExtent().Z - box.GetSize().Z - 100.f ) );
    //}

    {
      AShape* shape = Game->Make<AShape>( SHAPECUBE, Game->gm->neutralTeam );
      shape->SetSize( FVector( size.X, size.Y, 100.f ) );
      shape->SetPosition( FVector( 0, 0, box.Min.Z - box.GetExtent().Z - 100.f ) );
      shape->SetColor( FLinearColor::Yellow );
      AGroundPlane *f2 = GetWorld()->SpawnActor<AGroundPlane>( AGroundPlane::StaticClass() );
      f2->Mesh = shape->Mesh;
      shape->Mesh->AttachTo( f2->GetRootComponent(), NAME_None, EAttachLocation::KeepWorldPosition );
      shape->RemoveOwnedComponent( shape->Mesh );
      //f2->AddOwnedComponent( shape->Mesh ); // This is not allowed.., the owner is not f2
      // so you can't call AddOwnedComponent(), but on top of that 
      // Transferring a component from one actor to another is not allowed
      //shape->Dead = 1;
      //shape->MaxDeadTime = 2.f;
      //shape->Cleanup();
    }
  }

  // create the fog of war now
  ////fogOfWar = Game->Make<AFogOfWar>( FOGOFWAR, Game->gm->neutralTeam );
  ////fogOfWar->Init( floor->GetBox() );
}

void AFlyCam::MouseUpLeft()
{
  //LOG( "MouseUpLeft");
  Game->hud->MouseUpLeft( getMousePos() );
}

void AFlyCam::MouseDownLeft()
{
  // Covers all click behavior.
  if( Game->hud->MouseDownLeft( getMousePos() ) ) {
    return;
  }
}

void AFlyCam::MouseUpRight()
{
  
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

  FHitResult hit = Game->pc->RayPickSingle( getMousePos() );
  AGameObject* target = Cast<AGameObject>( hit.GetActor() );
  if( !target )
  {
    info( "Right clicked on nothing" );
    return;
  }

  // Hit the floor target, which means send units to ground position
  if( target == floor )
  {
    // Calculate offsets with respect to first command unit
    AGameObject* first = Game->hud->Selected[0];
    FVector offset = hit.ImpactPoint - first->Pos; // Offset to apply to get to pos from first->Pos
    for( AGameObject * go : Game->hud->Selected )
    {
      //go->GoToGroundPosition( go->Pos + offset ); // C++ Code Command
      if( Game->pc->IsAnyKeyDown( {EKeys::LeftShift, EKeys::RightShift } ) )
      {
        // When shift is down, we have to add the command to the list of commands for this unit.
        Game->EnqueueCommand( Command( Command::GoToGroundPosition, go->ID, go->Pos + offset ) ); // Network command
      }
      else
      {
        // When shift is NOT down, we have to clear the unit's command set.
        Game->SetCommand( Command( Command::GoToGroundPosition, go->ID, go->Pos + offset ) ); // Network command
      }
    }
  }
  else // Some object was right-clicked.
  {
    // An actor was hit by the click. Detect if friendly or not.
    for( AGameObject* go : Game->hud->Selected )
    {
      //go->Target( target );
      if( Game->pc->IsAnyKeyDown( { EKeys::LeftShift, EKeys::RightShift } ) )
      {
        Game->EnqueueCommand( Command( Command::Target, go->ID, target->ID ) );
      }
      else
      {
        Game->SetCommand( Command( Command::Target, go->ID, target->ID ) );
      }
    }
  }
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
          ghost->SetMaterialColors( "Multiplier", FLinearColor( .9,.2,.2,.5 ) );
        }
        else
        {
          // White to indicate placement success
          ghost->SetMaterialColors( "Multiplier", FLinearColor( 1.,1.,1.,.5 ) );
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


