#include "Wryv.h"
#include "TheHUD.h"
#include "FlyCam.h"
#include "Resource.h"
#include "PlayerControl.h"
#include "GameObject.h"
#include "Building.h"
#include "GlobalFunctions.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "Pathfinder.h"
#include "Peasant.h"
#include "Types.h"
#include "UISounds.h"
#include "GameFramework/PlayerInput.h"
#include "Runtime/Landscape/Classes/Landscape.h"

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
  // Make the fog of war instance.
  MovementComponent = PCIP.CreateDefaultSubobject<UFloatingPawnMovement>( this, ADefaultPawn::MovementComponentName );
  MovementComponent->SetTickableWhenPaused( true );
  CameraMovementSpeed = 1000.f;
}

// Called when the game starts or when spawned
void AFlyCam::BeginPlay()
{
	Super::BeginPlay();
  LOG( "FlyCam::BeginPlay()" );

  // We find ourselves inside the level of choice here.
  // From here, we decide what UI set to load.
  fogOfWar = GetWorld()->SpawnActor<AFogOfWar>( AFogOfWar::StaticClass() );
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
  MainCamera = GetComponentByName<UCameraComponent>( this, "Camera" );
  
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
  //music = UGameplayStatics::PlaySoundAttached( bkgMusic, RootComponent );
  sfxVolume = 1.f;
}

void AFlyCam::LoadLevel( FName levelName )
{
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
  // retrieves pointers to main objects for this frame
  // Frame setup. Runs at beginning of game frame.
  Game->hud->Setup(); // init selectors and renderers
  RetrievePointers();
  InitializePathfinding();

  // Construct the fog of war.
  setupLevel = 1;
}

void AFlyCam::InitializePathfinding()
{
  LOG( "Initializing pathfinding" );
  FindFloor();
  if( !floor ) {
    LOG( "InitializePathfinding(): No floor!" );
    return ; // means there's no floor! level cannot be played
  }
  // (or a floorplane can be constructed as a failsafe.

  // Initialize a bunch of bounding spheres
  if( !Rows )  Rows = Cols = 20;
  if( pathfinder ) delete pathfinder;
  pathfinder = new Pathfinder( Rows, Cols );
  
  FVector diff = floorBox.Max - floorBox.Min;
  vector<Types> intTypes;
  intTypes.push_back( Types::RESTREE );
  intTypes.push_back( Types::RESSTONE );
  intTypes.push_back( Types::RESGOLDMINE );
  
  // get the actual spacing between nodes, then divide by 2
  float radius = 0.95f*(1.f/Coord::Rows * diff.X)/2.f; // use 95% prox to ensure that
  // adjacent spheres don't overlap

  for( int i = 0; i < Coord::Rows; i++ )
  {
    for( int j = 0; j < Coord::Cols; j++ )
    {
      Coord coord( i, j );
      FVector perc( (float)i/Coord::Rows, (float)j/Coord::Cols, 0 );

      // For the landscape, the lower left is actually box.Min + ext.XY0.
      FVector ext = floorBox.GetExtent();
      ext.Z = 0.f;
      FVector ll = floorBox.Min;// + ext; // the center of the box is actually the lower left
      FVector p = perc*ll + (FVector(1.f, 1.f, 0.f) - perc)*floorBox.Max;
      p.Z = floorBox.Max.Z + radius*2.f; // move sphere above floor

      p = getHitFloor( p );

      int idx = coord.index();
      pathfinder->nodes[ idx ]->index = idx;
      pathfinder->nodes[ idx ]->point = p;
      
      AActor *sphere = MakeSphere( p, radius, White );
      
      // if the actor intersects any other bodies inside the game
      // then don't use it
      if( !intersectsAnyOfType( sphere, intTypes ) ) {
        pathfinder->nodes[ idx ]->terrain = Terrain::Passible;
        SetColor( sphere, White );
      }
      else {
        pathfinder->nodes[ idx ]->terrain = Terrain::Impassible;
        SetColor( sphere, Red );
      }

      pathfinder->updateGraphConnections( coord );
      sphere->SetActorScale3D( FVector( radius/2.f ) );

      if( !VizGrid )  sphere->Destroy(); // Don't show the sphere visualization
    }
  }
  
  if( !VizGrid )  return;

  // Visualization of graph
  set<Edge*> edges;
  for( int i = 0; i < pathfinder->nodes.size(); i++ )
  {
    GraphNode *node = pathfinder->nodes[i];
    //if( node->terrain == Passible )  MakeSphere( node->point, radius, White );
    // create a node and edge connections
    for( int j = 0; j < node->edges.size(); j++ )
      edges.insert( node->edges[j] );
  }

  for( Edge* e : edges )
  {
    FVector dir = e->dst->point - e->src->point;
    // set size of edge proportional to distance between nodes
    MakeLine( e->src->point, e->dst->point, White );
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
  LOG( "startLoc %f %f", perc.X, perc.Y );
  FVector P = GetActorLocation(); // restore the z value after movement
  FVector fwd = MainCamera->GetForwardVector();
  FBox box = floor->GetComponentsBoundingBox();

  // Find coordinates of click on ground plane
  // X & Y are reversed because Y goes right, X goes fwd.
  // Extents are HALF extents

  // Normal object (with centered origin)
  FVector G = box.Min + box.GetExtent() * 2.f * FVector( 1.f-perc.Y, perc.X, 0 );
  
  //FVector ext = box.GetExtent();
  //ext.Z = 0; // no z-measure
  //FVector G = (box.Min + ext) + ext * FVector( 1.f-perc.Y, perc.X, 0 );

  // for the ground when its not a plane
  //G.Z = box.Max.Z;

  float len = FVector::Dist( P, G );

  FVector P2 = G - len * fwd;
  P2.Z = P.Z; // nail z-value to old z-value to prevent camera climb
  
  // Stop the camera from previous motions
  //ControlInputVector = FVector::ZeroVector;

  // Move the camera back from the point on the plane in -cameraDir
  // vertically move the camera up to the higher plane
  // Move BACK in the direction of the fwd vector length of previous location
  //pos += camera->GetForwardVector() * -startLoc.Size();
  
  FQuat q( 0.f, 0.f, 0.f, 0.f );
  //LOG( "New camera loc %f %f %f", pos.X, pos.Y, pos.Z );
  SetActorLocationAndRotation( P2, q );

  // Compute frustum corners collision with ground plane.
  // find the frustum corners
  //FConvexVolume frustum;
  //GetViewFrustumBounds( frustum, this->camera->MarkRenderTransformDirty
}

FHitResult AFlyCam::LOS( FVector p, FVector q, TArray<AActor*> ignoredActors )
{
  FHitResult hit;
  FCollisionQueryParams fcqp( "dest trace", true );
  fcqp.AddIgnoredActors( ignoredActors );
  FCollisionObjectQueryParams fcoqp;
  GetWorld()->LineTraceSingleByObjectType( hit, p, q, fcoqp, fcqp );
  return hit;
}

void AFlyCam::SetColor( AActor* a, UMaterial* mat )
{
  vector<UMeshComponent*> meshes = GetComponentsByType<UMeshComponent>( a );
    for( int i = 0; i < meshes.size(); i++ )
      for( int j = 0; j < meshes[i]->GetNumMaterials(); j++ )
        meshes[i]->SetMaterial( j, mat );
}

void AFlyCam::Visualize( FVector& v, UMaterial* color )
{
  AActor *a = GetWorld()->SpawnActor( Game->unitsData[ Types::UNITSPHERE ].uClass, &v );
  a->GetRootComponent()->SetWorldScale3D( FVector( 10.f ) );
  viz.push_back( a );
}

void AFlyCam::Visualize( vector<FVector>& v )
{
  for( int i = 0; i < viz.size(); i++ )
    viz[i]->Destroy();
  viz.clear();

  for( int i = 0; i < v.size(); i++ )
  {
    LOG( "Pathway is (%f %f %f)", v[i].X, v[i].Y, v[i].Z );
    Visualize( v[i], Game->flycam->White );
  }

  // color the beginning and end 
  SetColor( viz.front(), Game->flycam->Green );
  SetColor( viz.back(), Game->flycam->Red );
}

AActor* AFlyCam::MakeSphere( FVector center, float radius, UMaterial* color )
{
  UClass *uClassSphere = Game->unitsData[ Types::UNITSPHERE ].uClass;
  AActor *sphere = GetWorld()->SpawnActor( uClassSphere, &center );
  SetColor( sphere, color );
  float s = sphere->GetComponentsBoundingBox().GetExtent().GetMax();
  if( !s )
  {
    LOG( "::MakeSphere(): %s had %f size bounding box", *sphere->GetName(), s );
    sphere->SetActorScale3D( FVector( radius ) );
  }
  else
    sphere->SetActorScale3D( FVector( radius/s ) );

  return sphere;
}

AActor* AFlyCam::MakeCube( FVector center, float radius, UMaterial* color )
{
  UClass *uClassSphere = Game->unitsData[ Types::UNITSPHERE ].uClass;
  AActor *cube = GetWorld()->SpawnActor( uClassSphere, &center );
  SetColor( cube, color );
  float s = cube->GetComponentsBoundingBox().GetExtent().GetMax();
  if( !s )
  {
    LOG( "::MakeCube(): %s had %f size bounding box", *cube->GetName(), s );
    cube->SetActorScale3D( FVector( radius ) );
  }
  else
    cube->SetActorScale3D( FVector( radius/s ) );

  return cube;
}

AActor* AFlyCam::MakeLine( FVector a, FVector b, UMaterial* color )
{
  // The line is a unit line
  FVector dir = b - a;
  float len = dir.Size();
  dir /= len;

  UClass *uClassEdge = Game->unitsData[ Types::UNITEDGE ].uClass;
  AActor *line = GetWorld()->SpawnActor( uClassEdge, &a );
  SetColor( line, color );
  line->SetActorScale3D( FVector( len ) );
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
}

void AFlyCam::debug( int slot, FColor color, FString mess )
{
	if( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( slot, 5.f, color, mess );
	}
}

void AFlyCam::setGhost( Types ut )
{
  FVector vec( 0. );

  // Delete the old ghost
  if( ghost )
  {
    // remove the old ghost and replace with a new one
    vec = ghost->Pos;
    ghost->Destroy();
    ghost = 0;
  }
  
  // If the buildingWidget was not set, then the ghost doesn't get created
  if( IsBuilding( ut ) )
  {
    // Remakes the ghost each frame a ghost is present.
    ghost = Game->Make( ut, vec, Game->gm->playersTeam->teamId );
  }
  else
  {
    LOG( "Cannot setGhost() to Nothing" );
  }
}

FVector2D AFlyCam::getMousePos()
{
  FVector2D mouse;
  Game->pc->GetMousePosition( mouse.X, mouse.Y ); 
  return mouse;
}

FHitResult AFlyCam::getHitGeometry()
{
  FHitResult hit;
  FVector2D mouse = getMousePos();
  // Trace into the scene and check what got hit.
  Game->pc->GetHitResultAtScreenPosition( mouse,
    ECollisionChannel::ECC_GameTraceChannel9, true, hit );
  return hit;
}

vector<FHitResult> AFlyCam::getAllHitGeometry()
{
  FVector2D mouse = getMousePos();
  vector<FHitResult> hits;
  Game->pc->TraceMulti( mouse, hits );
  return hits;
}

FVector AFlyCam::getHitFloor(FVector eye, FVector look)
{
  FHitResult hit;
  FCollisionQueryParams fcqp( "dest trace", true );
  bool intersects = floor->ActorLineTraceSingle( hit, eye, eye + look*1e6f, ECollisionChannel::ECC_GameTraceChannel9, fcqp );
  if( ! intersects )
  {
    warning( "Ray didn't hit ground plane" );
    return eye;
  }
  else
    return hit.ImpactPoint;
}

FVector AFlyCam::getHitFloor( FVector eye )
{
  return getHitFloor( eye, FVector( 0, 0, -1 ) );
}

FVector AFlyCam::getHitFloor()
{
  if( !floor )
  {
    LOG( "No floor");
    return FVector(0.f);
  }

  FVector2D mouse = getMousePos();
  FHitResult hit;
  Game->pc->Trace( mouse, floor, hit );
  return hit.ImpactPoint;
}

bool AFlyCam::intersectsAny( AActor* actor )
{
  vector<AActor*> except;
  return intersectsAny( actor, except );
}

bool AFlyCam::intersectsAny( AActor* actor, vector<AActor*>& except )
{
  // Check all actors in the level.
  ULevel* level = GetWorld()->GetLevel(0);
  TTransArray<AActor*> *actors = &level->Actors;
  for( int i = 0; i < actors->Num(); i++ )
  {
    // This is the actor we're checking for intersections with:
    AActor* a = (*actors)[i];
    if( a == nullptr || a == actor )
    {
      LOG( "null object found in Actors array");
      continue;
    }

    bool inExcept = 0;

    // Don't check actors in the except array.
    for( int j = 0; j < except.size() && !inExcept; j++ )
    {
      if( a == except[j] )
      {
        //LOG( "object %s excepted", *a->GetName() );
        inExcept = 1;
      }
    }
    if( inExcept )  continue;

    // Get the bounding box of the other actor.
    FBox box = a->GetComponentsBoundingBox();
    if( actor->GetComponentsBoundingBox().Intersect( box ) )
    {
      // candidate actor `a` and `actor` intersected.
      //LOG( "candidate actor %s intersected with %s",
      //  *a->GetName(), *actor->GetName() );
      return 1;
    }
  }
  
  //LOG( "didn't intersect any");
  return 0;
}

bool AFlyCam::intersectsAnyOfType( AActor* actor, vector<Types>& types )
{
  ULevel* level = GetWorld()->GetLevel(0);
  TTransArray<AActor*> *actors = &level->Actors;
  for( int i = 0; i < actors->Num(); i++ )
  {
    // This is the actor we're checking for intersections with:
    AGameObject* go = Cast<AGameObject>( (*actors)[i] );
    if( !go || go == actor )  continue;

    bool ok = 0;
    // Don't check actors in the except array.
    for( int j = 0; j < types.size() && !ok; j++ )
      if( go->Stats.Type == types[j] )
        ok = 1;
    if( !ok )  continue;

    // Get the bounding box of the other actor.
    FBox box = go->GetComponentsBoundingBox();
    if( actor->GetComponentsBoundingBox().Intersect( box ) )
      return 1;
  }
  
  return 0;
}

void AFlyCam::FindFloor()
{
  floor = 0;
  ULevel* level = GetWorld()->GetLevel(0);
  TTransArray<AActor*> *actors = &level->Actors;

  // First try and find a Landscape object representing the floor.
  for( int i = 0; i < actors->Num() && !floor; i++ )
  {
    AActor* a = (*actors)[i];
    if( !a )  continue;
    ALandscape* landscape = Cast< ALandscape >( a );
    if( landscape )  floor = a;
  }
  
  // Here, the floor wasn't found above, so search by name.
  if( !floor )
  {
    for( int i = 0; i < actors->Num() && !floor; i++ )
    {
      AActor* a = (*actors)[i];
      if( !a )  continue;
      if( a->GetName() == "floor" )  floor = a;
    }
  }

  if( !floor )  fatal( "Floor not found" ); // must crash because need floor for game to work
  floorBox = floor->GetComponentsBoundingBox();
}

void AFlyCam::Select( set<AGameObject*> objects )
{
  // Change UI to reflect selected object
  Game->hud->Select( objects );

  // Run its OnSelected function, playing sounds etc.
  for( AGameObject* go : objects )
    go->OnSelected();
  
}

void AFlyCam::MouseUpLeft()
{
  //LOG( "MouseUpLeft");
  Game->hud->MouseUpLeft( getMousePos() );
}

void AFlyCam::MouseDownLeft()
{
  // Since the FlyCam is the only PAWN in the game,
  // we must pass off the call to the HUD.
  // If the mouse click intersected a HUD element,
  // we don't let the click pass through to the 3d surface below it.
  if( Game->hud->MouseDownLeft( getMousePos() ) )  return;
  FUnitsDataRow NextSpell = Game->GetData( Game->hud->NextSpell );
  
  // If a spell queued to be cast, a left click casts it
  if( NextSpell.Type != NOTHING )
  {
    if( !Game->hud->Selected.size() ) return;  // can't cast the spell with no caster.

    // Cast the spell.
    // if the spell requires a target, check that we got one
    FHitResult hitResult = getHitGeometry();
    AGameObject* hit = Cast<AGameObject>( hitResult.GetActor() );
    if( hit ) { // Spell target
      // These cast on next turn, they can't cast from UI click directly
      // since engine needs to record cast event 
      for( AGameObject *se : Game->hud->Selected )
        se->CastSpell( NextSpell.Type, hit );
    }
    else if( NextSpell.AOE )
    {
      // Cast the spell on the ground 
      for( AGameObject *se : Game->hud->Selected )
        se->CastSpell( NextSpell.Type, getHitFloor() );
    }
    else
    {
      // Its not an AOE spell, and no target was hit by the click then error
      LOG( "Target was not selected" );
    }
  }
  else if( Game->hud->NextBuilding )
  {
    FUnitsDataRow NextBuilding = Game->GetData( Game->hud->NextBuilding );

    // Build the building. If ghost doesn't intersect any existing buildings then place it.
    vector<AActor*> except;
    except.push_back( ghost );
    except.push_back( floor );
    if( !intersectsAny( ghost, except ) )
    {
      // Otherwise, the building can be placed here spawn a copy
      // of the building, if the person has enough gold, lumber, stone to build it
      if( Game->gm->playersTeam->CanAfford( NextBuilding.Type ) )
      {
        Game->gm->playersTeam->Spend( Game->hud->NextBuilding );
        PlaySound( UISounds::BuildingPlaced );

        // It goes down as a little turf thing
        AGameObject* building = Game->Make( NextBuilding.Type, ghost->Pos, ghost->team->teamId );
        
        // Selected objects will go build it
        for( AGameObject* se : Game->hud->Selected )
        {
          // let the selected lastObject build the building
          if( APeasant* peasant = Cast<APeasant>( se ) )
          {
            // Send the peasant to build the building
            peasant->Build( Game->hud->NextBuilding, ghost->Pos );
          }
        }

        // Null last clicked widget so building can't be placed again unless selected
        Game->hud->NextBuilding = NOTHING;

        // leave the ghost where it was delete the ghost
        ghost->Destroy();
        ghost = 0;
      }
    }
  }
  
  
}

void AFlyCam::MouseUpRight()
{
  LOG( "MouseUpRight");
}

void AFlyCam::MouseDownRight()
{
  LOG( "MouseDownRight" );
  FHitResult hit = getHitGeometry();
  AGameObject* target = Cast<AGameObject>( hit.GetActor() );
  // There's a GameObject target
  if( target && target != floor )
  {
    // An actor was hit by the click
    for( AGameObject * go : Game->hud->Selected )
      go->SetTarget( target );
  }
  else
  {
    FVector loc = getHitFloor();
    for( AGameObject * go : Game->hud->Selected ) {
      LOG( "%s to %f %f %f", *go->GetName(), loc.X, loc.Y, loc.Z  );
      go->SetDestination( loc );
    }
  }
}

void AFlyCam::MouseMoved()
{
  //LOG( "AFlyCam::MouseMoved() frame %lld", Game->gm->tick );
  RetrievePointers();

  if( !setupLevel )
  {
    LOG( "Loading level" );
    OnLevelLoaded(); // This is here because it runs first for some reason (before ::Tick())
  }

  FVector2D mouse = getMousePos();
  Game->hud->MouseMoved( mouse );

  // if the mouse button is down, then its a drag event, elsee its a hover event
  if( Game->pc->IsKeyDown( EKeys::LeftMouseButton ) )
  {
    // 3D drag event, would be used for multiple object placement, or brushes.
  }
  else
  {
    // hover event. move the building ghost around etc.
    FHitResult hit = getHitGeometry();
  
    // If you're sliding the mouse along the floor,
    if( ghost && hit.Actor == floor )
    {
      ghost->SetActorLocation( hit.ImpactPoint );
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

void AFlyCam::MoveForward( float amount )
{
  // Gets called EACH FRAME (even if there's no input)
  if( Controller && amount )
  {
    FVector fwd = MainCamera->GetForwardVector();
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
    FVector back = -MainCamera->GetForwardVector();
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
    FVector left = -MainCamera->GetRightVector();
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
    FVector right = MainCamera->GetRightVector();
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


