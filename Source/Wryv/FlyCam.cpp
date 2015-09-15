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
#include "Projectile.h"
#include "GroundPlane.h"
#include "Types.h"
#include "UISounds.h"
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
  FloorBoxTraceFraction = 1.f;
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
}

// Called when the game starts or when spawned
void AFlyCam::BeginPlay()
{
  LOG( "AFlyCam::BeginPlay()" );
	Super::BeginPlay();
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

  TTransArray<AActor*> actors = GetWorld()->GetLevel(0)->Actors;
  // Set all resources on ground.
  for( int i = 0; i < actors.Num(); i++ )
  {
    AActor* a = actors[i];
    if( AResource* res = Cast<AResource>( a ) )
    {
      if( SetOnGround( res->Pos ) )
      {
        res->SetPosition( res->Pos ); // + FVector(0, 0, -25.f);
      }
      else
      {
        // The resource can't hit the ground
        LOG( "Resource %s cannot hit the ground plane", *res->Stats.Name );
      }
    }
  }

  // Construct the fog of war.
  setupLevel = 1;
}

void AFlyCam::InitializePathfinding()
{
  LOG( "Initializing pathfinding" );
  FindFloor();
  if( !floor ) {
    LOG( "InitializePathfinding(): No floor!" );
    return; // means there's no floor! level cannot be played
  }
  // (or a floorplane can be constructed as a failsafe).
  // Initialize a bunch of bounding spheres
  if( !Rows )  Rows = Cols = 20;
  if( pathfinder )  delete pathfinder;
  pathfinder = new Pathfinder( Rows, Cols );

  set<Types> intTypes;
  intTypes.insert( Types::RESLUMBER );
  intTypes.insert( Types::RESSTONE );
  intTypes.insert( Types::RESGOLD );
  
  // get the actual spacing between nodes, then divide by 2
  // use 95% prox to ensure that
  FVector radiusScale = 0.5f * CheckerSphereRadius * floorBox.GetSize() / FVector( Rows, Cols, 1.f );
  radiusScale.Z = radiusScale.X; // Keep it rounder

  // adjacent spheres don't overlap
  for( int row = 0; row < Rows; row++ )
  {
    for( int col = 0; col < Cols; col++ )
    {
      // Lay out the spheres
      FVector fraction( (row + .5f)/Rows, (col + .5f)/Cols, 1.f );

      // For the landscape, the lower left is actually box.Min + ext.XY0.
      FVector p = FMath::Lerp( floorBox.Min, floorBox.Max, fraction );
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
        AGameObject* sphere = Game->Make<AGameObject>( SHAPESPHERE, p, Game->gm->neutralTeam );
        sphere->SetSize( radiusScale );
        
        // Pick objects intersecting with the sphere. If anything intersects, then
        // the node is regarded as impassible.
        set<AGameObject*> forbidden = { floor };
        set<AGameObject*> intns = Game->pc->Pick( sphere, sphere->hitBounds ) | forbidden;
        
        if( intns.size() ) {
          pathfinder->nodes[ idx ]->terrain = Terrain::Impassible;
          sphere->SetColor( FLinearColor::Red );
        }
        else {
          pathfinder->nodes[ idx ]->terrain = Terrain::Passible;
          sphere->SetColor( FLinearColor::White );
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
      AGameObject *vizSphere = Game->Make<AGameObject>( SHAPESPHERE, node->point, Game->gm->neutralTeam );
      vizSphere->SetSize( radiusScale );
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
  FVector G = floorBox.Min + floorBox.GetExtent() * 2.f * FVector( 1.f-perc.Y, perc.X, 0 );
  
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
  //LOG( "New camera loc %f %f %f", pos.X, pos.Y, pos.Z );
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
  AGameObject* go = Game->Make<AGameObject>( type, v, Game->gm->neutralTeam );
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

AGameObject* AFlyCam::MakeLine( FVector a, FVector b, FLinearColor color )
{
  // The line is a unit line
  FVector dir = b - a;
  float len = dir.Size();
  if( !len ) {
    error( FS( "Making an edge of 0 length from %f %f %f to %f %f %f",
      a.X,a.Y,a.Z, b.X,b.Y,b.Z ) ) ;
    return 0;
  }
  dir /= len;

  AGameObject *line = Game->Make<AGameObject>( Types::SHAPEEDGE, a, Game->gm->neutralTeam );
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
  FVector2D mouse;
  Game->pc->GetMousePosition( mouse.X, mouse.Y ); 
  return mouse;
}

bool AFlyCam::SetOnGround( FVector& v )
{
  // Get ray hit with ground
  FVector v2 = v + FVector(0,0,floorBox.Max.Z);
  FHitResult hit = Game->pc->TraceAgainst( floor, v2, FVector( 0, 0, -1 ) );
  if( hit.GetActor() )
  {
    v = hit.ImpactPoint;
    return 1;
  }
  // no hit, no change
  warning( FS( "Point %f %f %f cannot hit the ground", v.X, v.Y, v.Z ) );
  return 0;
}

void AFlyCam::FindFloor()
{
  floor = 0;
  ULevel* level = GetWorld()->GetLevel(0);
  TTransArray<AActor*>& actors = level->Actors;

  // First try and find a Landscape object representing the floor.
  //for( int i = 0; i < actors.Num() && !floor; i++ )
  //{
  //  AActor* a = actors[i];
  //  if( !a )  continue;
  //  ALandscape* landscape = Cast< ALandscape >( a );
  //  if( landscape )  floor = a;
  //}
  
  // Here, the floor wasn't found above, so search by name.
  if( !floor )
  {
    for( int i = 0; i < actors.Num() && !floor; i++ )
    {
      if( AGroundPlane* gp = Cast<AGroundPlane>( actors[i] ) )
      {
        floor = gp;
      }
    }
  }

  if( !floor ) {
    error( "Floor not found" );
  }
  else {
    // trace on edges doesn't have issues
    floorBox = floor->GetComponentsBoundingBox();
    FloorBoxTraceFraction = FMath::Clamp( FloorBoxTraceFraction, 0.25f, 0.85f );
    floorBox.Min *= FloorBoxTraceFraction;
    floorBox.Max *= FloorBoxTraceFraction;
  }

  // create the fog of war now
  fogOfWar = GetWorld()->SpawnActor<AFogOfWar>( AFogOfWar::StaticClass() );
  fogOfWar->Init( floorBox );
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
  if( Game->hud->MouseDownLeft( getMousePos() ) ) {
    return;
  }

  // Check the HUD for what we're doing with the mouse click.
  FUnitsDataRow NextAction = Game->GetData( Game->hud->NextAction );
  
  // if the spell requires a target, check that we got one
  FHitResult hitResult = Game->pc->PickClosest( getMousePos() );

  // If we're doing something with the mouse click, do it here.
  if( NextAction.Type != NOTHING )
  {
    if( !Game->hud->Selected.size() ) return;  // can't cast the spell with no caster.
    
    AGameObject* hit = Cast<AGameObject>( hitResult.GetActor() );
    
    if( hit == floor )
    {
      // HUD-quesome attack onto the floor.
      if( NextAction.AttacksGround )
      {
        for( AGameObject *go : Game->hud->Selected )
        {
          /// Launch projectile @ ground
          AProjectile* p = Game->Make<AProjectile>( NextAction.Type, go->Pos, go->team );
          p->SetDestinationArc( go->Pos, hitResult.ImpactPoint, p->BaseStats.SpeedMax, p->BaseStats.MaxTravelHeight );
        }
      }
      //else // Commit the action
      //  for( AGameObject *se : Game->hud->Selected )
      //    se->Action( NextAction.Type, hit );
    }
    
    // We have performed the nextaction.
    Game->hud->NextAction = NOTHING;
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

  FHitResult hit = Game->pc->PickClosest( getMousePos() );
  AGameObject* target = Cast<AGameObject>( hit.GetActor() );
  if( !target )
  {
    info( FS( "Right clicked on nothing" ) );
    return;
  }

  // Hit the floor target, which means send units to ground position
  if( target == floor )
  {
    // Calculate offsets with respect to first command unit
    AGameObject* first = *Game->hud->Selected.begin();
    FVector offset = hit.ImpactPoint - first->Pos; // Offset to apply to get to loc from first->Pos
    for( AGameObject * go : Game->hud->Selected ) {
      go->SetGroundPosition( go->Pos + offset );
      //Game->EnqueueCommand( Command( Command::GoTo, go->ID, go->Pos + offset ) );
    }
  }
  else // Some object was right-clicked.
  {
    // An actor was hit by the click. Detect if friendly or not.
    for( AGameObject* go : Game->hud->Selected )
    {
      go->Target( target );
      //Game->EnqueueCommand( Command( Command::Target, target->ID ) );
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
    FHitResult hit = Game->pc->TraceAgainst( floor, mouse );
    
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


