#include "RTSGame.h"
#include "MyHUD.h"
#include "FlyCam.h"
#include "Resource.h"
#include "PlayerControl.h"
#include "GameObject.h"
#include "Building.h"
#include "GlobalFunctions.h"
#include "GroundPlane.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "Pathfinder.h"
#include "Peasant.h"
#include "Menu.h"
#include "DialogBox.h"
#include "TipsBox.h"

// Sets default values
AFlyCam::AFlyCam( const FObjectInitializer& PCIP ) : APawn( PCIP )
{
  // Set this character to call Tick() every frame.
  // Turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
  ghost = 0;
  floor = 0;
  setup = 0;

  dialogBox = 0;
  menu = 0;
  tipsBox = 0;

  MovementComponent = PCIP.CreateDefaultSubobject<UFloatingPawnMovement>(
    this, ADefaultPawn::MovementComponentName );
  movementSpeed = 1000.f;
}

void AFlyCam::SetupPlayerInputComponent( UInputComponent* InputComponent )
{
  check( InputComponent );
  Super::SetupPlayerInputComponent( InputComponent );
  InitializeDefaultPawnInputBindings();
  CameraZ = 2500.f;

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
  
  InputComponent->BindAction( "MouseClickedLMB", IE_Pressed, this, &AFlyCam::MouseLeftDown );
  InputComponent->BindAction( "MouseClickedLMB", IE_Released, this, &AFlyCam::MouseLeftUp );
  InputComponent->BindAction( "MouseClickedRMB", IE_Pressed, this, &AFlyCam::MouseRightDown );
  InputComponent->BindAction( "MouseClickedRMB", IE_Released, this, &AFlyCam::MouseRightUp );

  // Bind a key press to displaying the menu
  InputComponent->BindKey( EKeys::F10, IE_Pressed, this, &AFlyCam::DisplayMenu );
  //InputComponent->BindKey( EKeys::PageUp, IE_Pressed, this, &AFlyCam::MoveCameraZUp );
  //InputComponent->BindKey( EKeys::PageDown, IE_Pressed, this, &AFlyCam::MoveCameraZDown );
  FindFloor();
  //Setup();

  // Start the background music. Since we don't want attenuation,
  // we play the sound attached to the RootComponent of the Camera object,
  // and supply no further arguments.
  //music = UGameplayStatics::PlaySoundAttached( bkgMusic, RootComponent );
  sfxVolume = 1.f;

  // Construct the dialog boxes
  if( !dialogBox ) dialogBox = CreateWidget< UDialogBox >( GetWorld()->GetFirstPlayerController(), DialogBoxBlueprint );
  if( !menu ) menu = CreateWidget< UMenu >( GetWorld()->GetFirstPlayerController(), MenuBlueprint );
  if( !tipsBox ) {
    tipsBox = CreateWidget< UTipsBox >( GetWorld()->GetFirstPlayerController(), TipsBoxBlueprint );
    tipsBox->AddToViewport();
  }

  // Select a random tip
  tipNumber = randInt( 0, Tips.Num() );
  if( Tips.Num() )
    tipsBox->SetText( Tips[ tipNumber ] );
}

void AFlyCam::InitializeDefaultPawnInputBindings()
{
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
}

void AFlyCam::MoveCameraZUp( float amount )
{
  //UE_LOG( LogTemp, Warning, TEXT("MoveCameraZUp(%f)"), amount );
  if( Controller && amount )
  {
    FVector up( 0, 0, 1 );
    AddMovementInput( up, movementSpeed*amount );
  }
}

void AFlyCam::MoveCameraZDown( float amount )
{
  //UE_LOG( LogTemp, Warning, TEXT("MoveCameraZDown(%f)"), amount );
  if( Controller && amount )
  {
    FVector down( 0, 0, -1 );
    AddMovementInput( down, movementSpeed*amount );
  }
}

void AFlyCam::SetCameraPosition( FVector2D perc )
{
  FVector startLoc = GetActorLocation();
  FBox box = floor->GetComponentsBoundingBox();
  FVector pos = box.Min;
  
  // X & Y are reversed because Y goes right, X goes fwd.
  // Extents are HALF extents
  pos += box.GetExtent() * 2.f * FVector( 1.f-perc.Y, perc.X, 0 );
  
  // Move the camera back from the point on the plane in -cameraDir
  // vertically move the camera up to the higher plane
  // Move BACK in the direction of the fwd vector length of previous location
  pos += camera->GetForwardVector() * -startLoc.Size();

  FQuat q( 0.f, 0.f, 0.f, 0.f );
  SetActorLocationAndRotation( pos, q );
}

void AFlyCam::NextTip()
{
  tipNumber++;
  if( Tips.Num() )
  {
    tipNumber %= Tips.Num();
    tipsBox->SetText( Tips[ tipNumber ] );
  }
}

void AFlyCam::DisplayMenu()
{
  UE_LOG( LogTemp, Warning, TEXT("DisplayMenu()") );
  menu->AddToViewport();
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
  vector<UMeshComponent*> meshes = AGameObject::GetComponentsByType<UMeshComponent>( a );
    for( int i = 0; i < meshes.size(); i++ )
      for( int j = 0; j < meshes[i]->GetNumMaterials(); j++ )
        meshes[i]->SetMaterial( j, mat );
}

void AFlyCam::Visualize( FVector& v, UMaterial* color )
{
  AActor *a = GetWorld()->SpawnActor(
    Game->unitsData[ Types::UNITSPHERE ].uClass, &v );
  a->GetRootComponent()->SetWorldScale3D( FVector( 100.f ) );
  viz.push_back( a );
}

void AFlyCam::Visualize( vector<FVector>& v )
{
  for( int i = 0; i < viz.size(); i++ )
    viz[i]->Destroy();
  viz.clear();

  for( int i = 0; i < v.size(); i++ )
  {
    UE_LOG( LogTemp, Warning, TEXT("Pathway is (%f %f %f)"),
      v[i].X, v[i].Y, v[i].Z );
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
    UE_LOG( LogTemp, Warning, TEXT( "::MakeSphere(): %s had %f size bounding box" ), *sphere->GetName(), s );
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
    UE_LOG( LogTemp, Warning, TEXT( "::MakeCube(): %s had %f size bounding box" ), *cube->GetName(), s );
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

void AFlyCam::InitLevel()
{
  FBox box = floor->GetComponentsBoundingBox();
  
  // Initialize a bunch of bounding spheres
  if( !Rows )  Rows = Cols = 20;
  pathfinder = new Pathfinder( Rows, Cols );
  
  FVector diff = box.Max - box.Min;
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
      FVector p = perc*box.Min + (FVector(1.f, 1.f, 1.f) - perc)*box.Max;
      p.Z = box.Max.Z + radius;

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

void AFlyCam::Setup()
{
  // Frame setup. Runs at beginning of game frame.
  Game->pc = Cast<APlayerControl>( GetWorld()->GetFirstPlayerController() );
  HotSpot::hud = Game->myhud = Cast<AMyHUD>( Game->pc->GetHUD() );
  Game->gm = (ARTSGameGameMode*)GetWorld()->GetAuthGameMode();
  Game->flycam = this;

  // The pitch of the camera is setup in the game blueprint.
  // we capture the camera component here
  camera = GetComponentByName<UCameraComponent>( this, "Camera" );

  // Initialize the data table.
  Game->Init();
  
  if( !setup )
  {
    Game->myhud->Setup();
    InitLevel(); // create the "gutter"
    setup = 1;
  }
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
    vec = ghost->Pos();
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
    //UE_LOG( LogTemp, Warning, TEXT( "Cannot setGhost() to Nothing" ) );
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

FVector AFlyCam::getHitFloor()
{
  if( !floor )
  {
    UE_LOG( LogTemp, Warning, TEXT("No floor") );
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
      UE_LOG( LogTemp, Warning, TEXT("null object found in Actors array") );
      continue;
    }

    bool inExcept = 0;

    // Don't check actors in the except array.
    for( int j = 0; j < except.size() && !inExcept; j++ )
    {
      if( a == except[j] )
      {
        //UE_LOG( LogTemp, Warning, TEXT("object %s excepted"), *a->GetName() );
        inExcept = 1;
      }
    }
    if( inExcept )  continue;

    // Get the bounding box of the other actor.
    FBox box = a->GetComponentsBoundingBox();
    if( actor->GetComponentsBoundingBox().Intersect( box ) )
    {
      // candidate actor `a` and `actor` intersected.
      //UE_LOG( LogTemp, Warning, TEXT("candidate actor %s intersected with %s"),
      //  *a->GetName(), *actor->GetName() );
      return 1;
    }
  }
  
  //UE_LOG( LogTemp, Warning, TEXT("didn't intersect any") );
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
      if( go->UnitsData.Type == types[j] )
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
  ULevel* level = GetWorld()->GetLevel(0);
  TTransArray<AActor*> *actors = &level->Actors;
  for( int i = 0; i < actors->Num() && !floor; i++ )
  {
    AActor* a = (*actors)[i];
    if( a == nullptr ) // This happens a lot
    {
      UE_LOG( LogTemp, Warning, TEXT("Null object found in Actors array") );
      continue;
    }
    
    //UE_LOG( LogTemp, Warning, TEXT("Actor: %s"), *a->GetName() );
    // This is the actor we're checking for intersections with:
    // Cast to ground plane to check if the object is a GroundPlane object.
    if( a->IsA<AGroundPlane>() )
    {
      //UE_LOG( LogTemp, Warning, TEXT("Found the floor `%s`"), *a->GetName() );
      floor = (AGroundPlane*)a;
    }
  }
  
  if( !floor )
  {
    UE_LOG( LogTemp, Warning, TEXT("floor object not found") );
    return;
  }
}

void AFlyCam::MouseLeftUp()
{
  UE_LOG( LogTemp, Warning, TEXT("MouseLeftUp") );
  Game->myhud->MouseLeftUp( getMousePos() );
}

void AFlyCam::MouseLeftDown()
{
  UE_LOG( LogTemp, Warning, TEXT("MouseLeftDown") );
  
  // First, test for intersect with UI
  FVector2D mouse = getMousePos();

  AGameObject* lo = Game->myhud->SelectedObject;
  
  // Check if the mouse was clicked on the HUD.
  // If the wood panel was clicked, we would enter here as well,
  if( Game->myhud->MouseLeftDown( mouse ) )
  {
    // The HUD was clicked. The lastClickedWidget
    // is the building that was selected for placement.
    // Can be NULL if no building button was pushed.
    setGhost( Game->myhud->NextBuilding );

    // The mouse hit something on the HUD, so we don't trace into the scene
    return;
  }
  
  // Get the geometry that was hit.
  FHitResult hitResult = getHitGeometry();
  AGameObject* hit = Cast<AGameObject>( hitResult.GetActor() );
  if( hit )
  {
    UE_LOG( LogTemp, Warning, TEXT("Clicked on %s"), *hit->UnitsData.Name );
  }
  else
  {
    UE_LOG( LogTemp, Warning, TEXT("Nothing was selected") );
    return;
  }

  // The hit object won't be the floor, it'll be the
  // "ghost" object (object being placed)
  // 
  // If the click was on the floor, place a building only if the
  // ghost doesn't intersect with any other object.
  // Place a building if not blocked
  //UE_LOG( LogTemp, Warning, TEXT("Place building") );
  if( lo )
  {
    // If a spell is queued, cast it @ hit object.
    if( Game->myhud->NextSpell )
    {
      lo->CastSpell( Game->myhud->NextSpell, hit );
      Game->myhud->NextSpell = NOTHING;
    }
    else if( Game->myhud->NextBuilding ) // A building is attempted to being placed by selected peasant
    {
      // If ghost doesn't intersect any existing buildings then place it.
      vector<AActor*> except;
      except.push_back( ghost );
      except.push_back( floor );
      if( !intersectsAny( ghost, except ) )
      {
        // Otherwise, the building can be placed here spawn a copy
        // of the building, if the person has enough gold, lumber, stone to build it
        if( Game->gm->playersTeam->CanAfford( Game->myhud->NextBuilding ) )
        {
          if( APeasant *p = Cast<APeasant>( Game->myhud->SelectedObject ) )
          {
            UGameplayStatics::PlaySoundAttached( buildingPlaced, RootComponent );
            Game->gm->playersTeam->Spend( Game->myhud->NextBuilding );
            
            // Place the building in the position indicated
            // let the selected lastObject build the building
            //lo->NextBuilding = Game->myhud->NextBuilding;
            p->Build( Game->myhud->NextBuilding, ghost->Pos() );

            // Null last clicked widget so building can't be
            // placed again unless selected
            Game->myhud->NextBuilding = NOTHING;

            // leave the ghost where it was
            // delete the ghost
            ghost->Destroy();
            ghost = 0;
          }
        }
      }
    }
  }
  
  // When not placing a building OR casting a spell, we're picking something.
  // from the screen whose info should be displayed in the sidebar.
  UE_LOG( LogTemp, Warning, TEXT("Clicked on %s"), *hit->UnitsData.Name );
  
  // Here we check if we need to change the selected unit  
  // Change selected object, as long as a spell wasn't queued on LO
  if( hit != floor )
  {
    // if there is a lastObject WITH a spell queued, then don't change the selected object.
    if( lo   &&   lo->NextSpell )
    {
      // Setting spell target to the object that was hit
    }
    else
    {
      Game->myhud->SelectedObject = hit;
      hit->OnSelected();
    }
  }

  ///////////////////////////
  // position on lastClicked object is basically where on the object
  // the building is to be placed.
  //UE_LOG( LogTemp, Warning, TEXT("%f %f %f"), hitPos.X, hitPos.Y, hitPos.Z );
}

void AFlyCam::MouseRightDown()
{
  UE_LOG( LogTemp, Warning, TEXT("MouseRightDown") );
  AGameObject *lo = Game->myhud->SelectedObject;
  FHitResult hit = getHitGeometry();
  if( hit.Actor != floor )
  {
    // An actor was hit by the click
    if( lo )
    {
      lo->SetTarget( Cast<AGameObject>( hit.Actor.Get() ) );
    }
  }
  else
  {
    // Send unit to a specific location on the floor.
    FVector loc = getHitFloor();
    UE_LOG( LogTemp, Warning, TEXT("Right click occurred @ (%f, %f, %f)"),
      loc.X, loc.Y, loc.Z );
    if( lo )
    {
      lo->SetDestination( loc );
    }
  }
}

void AFlyCam::MouseRightUp()
{
  UE_LOG( LogTemp, Warning, TEXT("MouseRightUp") );
  
}

void AFlyCam::MouseMoved()
{
  //UE_LOG( LogTemp, Warning, TEXT("MouseMoved() frame %d"), Game->tick );
  Setup(); // This is here because it runs first for some reason (before ::Tick())
  FVector2D mouse = getMousePos();
  Game->myhud->MouseMoved( mouse );

  // if the mouse button is down, then its a drag event, elsee its a hover event
  bool leftMouseDown = Game->pc->IsDown( EKeys::LeftMouseButton );
  if( leftMouseDown )
  {
    // drag event

  }
  else
  {
    // hover event
  }

  FHitResult hit = getHitGeometry();
  
  // If you're sliding the mouse along the floor,
  if( ghost && hit.Actor == floor )
  {
    ghost->SetActorLocation( hit.ImpactPoint );
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

void AFlyCam::MoveForward( float amount )
{
  //MoveForward( 2.f );

  // Don't enter the body of this function if Controller is
  // not set up yet, or if the amount to move is equal to 0 
  if( Controller && amount )
  {
    FVector fwd( 1, 0, 0 );
    AddMovementInput( fwd, movementSpeed*amount );
  }
}

void AFlyCam::MoveBack( float amount )
{
  if( Controller && amount )
  {
    //UE_LOG( LogTemp, Warning, TEXT("MoveBack %f"), amount );
    FVector back( -1, 0, 0 );
    AddMovementInput( back, movementSpeed*amount );
  }
}

void AFlyCam::MoveLeft( float amount )
{
  if( Controller && amount )
  {
    //UE_LOG( LogTemp, Warning, TEXT("MoveLeft %f"), amount );
    FVector left( 0, -1, 0 );
    AddMovementInput( left, movementSpeed*amount );
  }
}

void AFlyCam::MoveRight( float amount )
{
  if( Controller && amount )
  {
    //UE_LOG( LogTemp, Warning, TEXT("MoveRight %f"), amount );
    FVector right( 0, 1, 0 );
    AddMovementInput( right, movementSpeed*amount );
  }
}

// Called when the game starts or when spawned
void AFlyCam::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AFlyCam::Tick( float t )
{
	Super::Tick( t );
}


