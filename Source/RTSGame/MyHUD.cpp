#include "RTSGame.h"
#include "FlyCam.h"
#include "Resource.h"
#include "Building.h"
#include "PlayerControl.h"
#include "MyHUD.h"
#include "Unit.h"
#include "ItemShop.h"
#include "UnitsData.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "Spell.h"
#include "GlobalFunctions.h"
#include "Widget.h"
#include "DrawDebugHelpers.h"

#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
#include "Editor/UnrealEd/Public/AssetThumbnail.h"

AMyHUD::AMyHUD(const FObjectInitializer& PCIP) : Super(PCIP)
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::AMyHUD(ctor)") );
  selector = 0;
  selectorAttackTarget = 0;
  selectorShopPatron = 0;
  SelectedObject = 0;
  
  Init = 0; // Have the slots & widgets been initialied yet? can only happen
  // in first call to draw.
}

void AMyHUD::BeginPlay()
{
  Super::BeginPlay();
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::BeginPlay()") );
  LoadUClasses();
}

TArray<FAssetData> AMyHUD::ScanFolder( FName folder )
{
  // Create an AssetRegistry to list folder data
  FAssetRegistryModule& AssetRegistry =
    FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
    FName("AssetRegistry"));
  
  // Switch into the SubPaths folder. Required for 
  // GetAssetsByPath to work properly (even if we don't use the pathList)
  TArray<FString> pathList;
  AssetRegistry.Get().GetSubPaths( folder.ToString(), pathList, 1 );
  TArray<FAssetData> assets;
  AssetRegistry.Get().GetAssetsByPath( folder, assets );

  for( int i = 0; i < assets.Num(); i++ )
  {
    UE_LOG( LogTemp, Warning, TEXT("%s"), *assets[i].AssetName.ToString() );
  }
  return assets;
}

void AMyHUD::InitWidgets()
{
  if( Init ) return;
  Init = 1;

  UE_LOG( LogTemp, Warning, TEXT( "InitWidgets()" ) );

  // Initialize the widgets that show the player gold, lumber, stone counts.
  ResourcesWidget::GoldTexture = GoldIconTexture;
  ResourcesWidget::LumberTexture = LumberIconTexture;
  ResourcesWidget::StoneTexture = StoneIconTexture;

  SolidWidget::SolidWhiteTexture = SolidWhiteTexture;
  SlotPalette::SlotPaletteTexture = SlotPaletteTexture;
  StackPanel::StackPanelTexture = StackPanelTexture;
  ui = new UserInterface(FVector2D(Canvas->SizeX, Canvas->SizeY));
  GameChrome *gChrome = ui->gameChrome;

  // attach selectBox manipulation functions to ui's function handlers
  ui->OnMouseDownLeft = [this]( FVector2D mouse ){
    // Set the opening of the select box
    ui->selectBox->SetStart( mouse );
    ui->selectBox->Show();
    return NotConsumed;
  };

  ui->OnHover = [this]( FVector2D mouse ){
    // Regular hover event
    return NotConsumed;
  };
  // Mouse motion with mouse down.
  ui->OnMouseDragLeft = [this]( FVector2D mouse ){
    ui->selectBox->SetEnd( mouse );
    ui->selectBox->Show();
    return NotConsumed;
  };
  // Mouse up event, after having Dragged 
  ui->OnMouseUpLeft = [this]( FVector2D mouse ){
    // form selection of elements contained in box & hide box select
    ui->selectBox->Hide();
    return NotConsumed;
  };

  gChrome->rightPanel = new SidePanel( RightPanelTexture, PortraitTexture,
    MinimapTexture, FVector2D( 280, Canvas->SizeY ), FVector2D(8,8) );
  gChrome->rightPanel->Align = TopRight;
  gChrome->Add( gChrome->rightPanel );
  
  Controls *controls = gChrome->controls = new Controls( PauseButtonTexture );
  gChrome->rightPanel->Add( controls );
  controls->Align = Top | ToLeftOfParent;
  
  // flush top with 0 top of parent
  controls->Margin.Y = - gChrome->rightPanel->Pad.Y;
  controls->Margin.X = 4;
  controls->Pause->OnMouseDownLeft = [this,controls]( FVector2D mouse ){
    Game->pc->SetPause( !Game->pc->IsPaused() );  // pauses or unpauses the game
    if( Game->pc->IsPaused() )
      controls->Pause->Tex = ResumeButtonTexture;
    else
      controls->Pause->Tex = PauseButtonTexture;
    return 0;
  };
  
  // Attach functionality for minimap
  Minimap* minimap = gChrome->rightPanel->minimap;
  minimap->OnMouseDownLeft = [minimap](FVector2D mouse){
    Game->flycam->SetCameraPosition( mouse / minimap->Size );
    return Consumed;
  };
  minimap->OnMouseDragLeft = [minimap](FVector2D mouse){
    Game->flycam->SetCameraPosition( mouse / minimap->Size );
    return Consumed;
  };

  // Keep one of these for showing costs on flyover
  gChrome->costWidget = new CostWidget( TooltipBackgroundTexture );
  gChrome->Add( gChrome->costWidget );
  gChrome->costWidget->Align = CenterCenter;

  gChrome->tooltip = new Tooltip( TooltipBackgroundTexture, FString("tooltip") );
  gChrome->tooltip->Pad = FVector2D( 8, 8 );
  gChrome->Add( gChrome->tooltip );
  gChrome->tooltip->Hide();

  gChrome->buffs = new StackPanel( "Buffs", 0 );
  gChrome->buffs->Pad = FVector2D( 4, 4 );
  gChrome->Add( gChrome->buffs );

  gChrome->buildQueue = new StackPanel( "Building Queue", 0 );
  gChrome->buildQueue->Pad = FVector2D( 4, 4 );
  gChrome->Add( gChrome->buildQueue );

  // Create the panel for containing items/inventory
  gChrome->itemBelt = new SlotPalette( SlotPaletteTexture, 1, 4, 
    FVector2D( 100, 100 ), FVector2D( 8, 8 ) );
  gChrome->Add( gChrome->itemBelt );
  gChrome->itemBelt->Align = BottomCenter;

  // Map selection screen
  MapSelectionScreen *mss = ui->mapSelectionScreen = 
    new MapSelectionScreen( TitleNameTexture, SolidWhiteTexture,
      MapSlotEntryBackgroundTexture, PortraitTexture,
      FVector2D( 120, 24 ), largeFont );

  // Construct the other screens
  ui->titleScreen = new Title( TitleScreenTexture );
  
  ui->mapSelectionScreen->Align = CenterCenter;
  ui->Add( ui->mapSelectionScreen );
  ui->mapSelectionScreen->OKButton->OnMouseDownLeft = [mss](FVector2D mouse){
    // OK button clicked, so load the map if there is a selected widget
    // else display error message
    if( mss->Selected )
      Game->flycam->LoadLevel( FName( *mss->Selected->GetText() ) );
    else
      Game->myhud->ui->statusBar->Set( "Select a map to load first" ) ;
    return NotConsumed;
  };

  /////
  // List the maps in the folder at the left side
  TArray<FAssetData> maps = ScanFolder( "/Game/Maps" );

  for( int i = 0; i < maps.Num(); i++ )
    ui->mapSelectionScreen->AddText( maps[i].AssetName.ToString(), CenterCenter );

  ui->missionObjectivesScreen = new MissionObjectivesScreen(
    MapSlotEntryBackgroundTexture, MapSlotEntryBackgroundTexture, 
    FVector2D( 300, 100 ), FVector2D( 8, 8 ) );
  ui->Add( ui->missionObjectivesScreen );

  // Add the mouseCursor last so that it renders last.
  ui->mouseCursor = new ImageWidget( MouseCursorHand.Texture );
  ui->Add( ui->mouseCursor );

  /// Set the screen's to correct one for the gamestate
  ui->SetScreen( Game->gm->state );

}

void AMyHUD::LoadUClasses()
{
  // Connect Widgets & UnitsData with mappings from Types
  vector< Types > types;
  for( int i = 0; i < Types::MAX; i++ )  types.push_back( (Types)i );
  
  // Pull all the UCLASS names from the Blueprints we created.
  for( int i = 0; i < UnitTypeUClasses.Num(); i++ )
  {
    Types ut = UnitTypeUClasses[i].Type;
    UClass* uc = UnitTypeUClasses[i].uClass;
    if( !uc )  continue;
    UE_LOG( LogTemp, Warning, TEXT("Loaded unit=%s / UClass=%s" ),
      *GetEnumName( ut ), *uc->GetClass()->GetName() );
    
    // create a unit of that type from the blueprint to extract the properties
    // that were entered inside blueprints
    AGameObject* unit = (AGameObject*)GetWorld()->SpawnActor( uc );
    if( ! unit )
    {
      UE_LOG( LogTemp, Warning, TEXT("  Couldn't load: %s" ), *uc->GetClass()->GetName() );
      continue;
    }

    // Save in all the defaults that blueprint specifies,
    // including spawn costs, etc. We need this data here
    // because we need to check if we have enough money
    // to spawn a unit of a certain type before spawning it.
    unit->UnitsData.uClass = uc;  // Not available in dropdowns from table selector.
    Game->unitsData[ ut ] = unit->UnitsData;
    widgets[ ut ] = unit->Widget; // Save copies of each widget in Widgets array for reference.
    
    //UE_LOG( LogTemp, Warning, TEXT("Loaded unit: %s" ), *unit->UnitsData.ToString() );
    unit->Destroy(); // destroy the unit 
  }

  int j = 0;
  for( pair<const Types, FWidgetData>& p : widgets )
  {
    UE_LOG( LogTemp, Warning, TEXT("Entry %d:  Tex: %s, %d" ),
      j, *p.second.Label, (int)p.second.Type.GetValue() );
    j++;
  }
}

void AMyHUD::Setup()
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::Setup()" ) );

  FVector v(0.f);
  FRotator r(0.f);
  if( !selector )  selector = GetWorld()->SpawnActor<AActor>( uClassSelector, v, r );
  if( !selectorAttackTarget )  selectorAttackTarget = GetWorld()->SpawnActor<AActor>( uClassSelectorA, v, r );
  if( !selectorShopPatron )  selectorShopPatron = GetWorld()->SpawnActor<AActor>( uClassSelectorShop, v, r );

  rendererIcon = GetComponentByName<USceneCaptureComponent2D>( this, "rendererIcon" ); //rs[0];
  rendererMinimap = GetComponentByName<USceneCaptureComponent2D>( this, "rendererMinimap" ); //rs[1];
}

void AMyHUD::SetAttackTargetSelector( AGameObject* target )
{
  if( target )
  {
    FVector v = SelectedObject->attackTarget->Pos();
    if( Game->flycam->floor )
    {
      //UE_LOG( LogTemp, Warning, TEXT("-- %s"), *Game->flycam->floor->UnitsData.Name );
      FBox box = Game->flycam->floor->GetComponentsBoundingBox();
      v.Z = box.Min.Z;
    }
    selectorAttackTarget->SetActorLocation( v );
  }
  else
  {
    // Put the ring out somewhere else
    selectorAttackTarget->SetActorLocation( FVector(0.f) );
  }
}

void AMyHUD::SetShopTargetSelector( AGameObject* target )
{
  if( target )
  {
    if( AItemShop *is = Cast<AItemShop>( SelectedObject ) )
    {
      FVector v = is->patron->Pos();
      selectorShopPatron->SetActorLocation( v );
    }
  }
  else
  {
    // Put the ring out somewhere else
    selectorShopPatron->SetActorLocation( FVector(0.f) );
  }
}

void AMyHUD::UpdateSelectedObjectStats()
{
  // "Prints" the selected object's:
  //   1. rendered portrait
  //   2. text stats
  //   3. buffs
  //   4. spawnQueue
  // the rightPanel object.
  if( SelectedObject )
  {
    // 1. Portrait:
    // render last-clicked object to texture
    // zoom back by radius of bounding sphere of clicked object
    FVector camDir( .5f, .5f, -FMath::Sqrt( 2.f ) );
    RenderScreen( rendererIcon, PortraitTexture, SelectedObject->Pos(), SelectedObject->GetBoundingRadius(), camDir );
    
    // 2. Text stats:
    // This is for the picture of the last clicked object. Generate a widget for the picture of the unit.
    // Print unit's stats into the stats panel
    ui->gameChrome->rightPanel->unitStats->Set( SelectedObject->PrintStats() );
    ui->gameChrome->rightPanel->portrait->Tex = SelectedObject->Widget.Tex;

    // 3. buffs. PER-FRAME: Clear & then re-draw the buffs
    ui->gameChrome->buffs->Clear(); // Clear any existing/previous buffs.
    if( AUnit* unit = Cast<AUnit>( SelectedObject ) )
    {
      // Go through the applied buffs
      for( int i = 0; i < unit->BonusTraits.size(); i++ )
      {
        Types buff = unit->BonusTraits[i].traits.Type;
        ui->gameChrome->buffs->StackRight( new ImageWidget( Game->myhud->widgets[ buff ].Tex ) );
      }
    }

    // 4. Clear & re-draw the spawn queue
    // Draw icons for the objects being spawned, and their progress.
    ui->gameChrome->buildQueue->Clear();
    SelectedObject->spawnQueue.clear();
    SelectedObject->spawnQueue.push_back( SpawningObject( 8.f, Types::UNITFOOTMAN ) );
    SelectedObject->spawnQueue.push_back( SpawningObject( 8.f, Types::UNITFOOTMAN ) );
    for( int i = 0; i < SelectedObject->spawnQueue.size(); i++ )
    {
      SpawningObject so = SelectedObject->spawnQueue[i];
      ImageWidget *img = new ImageWidget( Game->myhud->widgets[ so.Type ].Tex );
      img->Color.A = so.Percent();
      ui->gameChrome->buildQueue->StackRight(img);
      // Draw a cooldownpie on top of each node
      //img->Add( new CooldownPie( so.time ) );
    }
  }
}

void AMyHUD::UpdateDisplayedResources()
{
  // spent=200, diff=-200
  //    diff = 200 - 400;
  float diff = Game->gm->playersTeam->Gold - displayedGold;
  diff *= 0.1; // jump by 10%
  displayedGold += diff;

  diff = Game->gm->playersTeam->Lumber - displayedLumber;
  diff *= 0.1; // jump by 10%
  displayedLumber += diff;

  diff = Game->gm->playersTeam->Stone - displayedStone;
  diff *= 0.1; // jump by 10%
  displayedStone += diff;

  ui->gameChrome->resources->SetValues( FMath::RoundToInt( displayedGold ),
    FMath::RoundToInt( displayedLumber ), FMath::RoundToInt( displayedStone ) );
}

void AMyHUD::UpdateMouse()
{
  // Draw the mouse
  ui->mouseCursor->Margin = Game->flycam->getMousePos();
  // If the `NextSpell` is selected, cross hair is used, else hand.
  if( NextSpell ) {
    ui->mouseCursor->Tex = MouseCursorCrossHairs.Texture;
    ui->mouseCursor->hotpoint = MouseCursorCrossHairs.Hotpoint;
  }
  else {
    ui->mouseCursor->Tex = MouseCursorHand.Texture;
    ui->mouseCursor->hotpoint = MouseCursorHand.Hotpoint;
  }
}

void AMyHUD::DrawHUD()
{
  // Canvas is only initialized here.
  Super::DrawHUD();
  HotSpot::hud = this;

  InitWidgets();

  UpdateDisplayedResources();
  UpdateSelectedObjectStats();
  UpdateMouse();
  
  // Render the minimap, only if the floor is present
  if( Game->flycam->floor ) {
    FBox box = Game->flycam->floor->GetComponentsBoundingBox();
    FVector p = box.GetCenter();
    RenderScreen( rendererMinimap, MinimapTexture, p, box.GetExtent().GetMax(), FVector( 0, 0, -1 ) );
  }

  // refresh the size of the ui the layout
  ui->Size = FVector2D( Canvas->SizeX, Canvas->SizeY );

  // Render the entire UI
  ui->render();

  // Render the cinematic from the material (must be setup to use
  // an Emissive channel)
  //DrawMaterial( MediaMaterial, 0, 0,
  //  Canvas->SizeX, Canvas->SizeY, 0, 0, 1, 1, 1.f );
}

// Detect clicks and mouse moves on the HUD
HotSpot* AMyHUD::MouseDownLeft( FVector2D mouse )
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::MouseDownLeft()") );
  return ui->MouseDownLeft( mouse );
}

HotSpot* AMyHUD::MouseUpLeft( FVector2D mouse )
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::MouseUpLeft()") );
  return ui->MouseUpLeft( mouse );
}

HotSpot* AMyHUD::MouseDownRight( FVector2D mouse )
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::MouseDownRight()") );
  return ui->MouseDownRight( mouse );
}

HotSpot* AMyHUD::MouseUpRight( FVector2D mouse )
{
  // Right up isn't handled.
  return ui->MouseUpRight( mouse );
}

HotSpot* AMyHUD::MouseMoved( FVector2D mouse )
{
  if( !Init )  return 0;

  // Drag events are when the left mouse button is down.
  if( Game->pc->IsDown( EKeys::LeftMouseButton ) )
  {
    return ui->MouseDraggedLeft( mouse );
  }
  else
  {
    // Check against all ui widgets that require hover.
    return ui->Hover( mouse );
  }

  return 0;
}

void AMyHUD::Tick( float t )
{
  if( selector   &&   SelectedObject )
  {
    FVector v = SelectedObject->Pos();
    FBox box = Game->flycam->floor->GetComponentsBoundingBox();
    v.Z = box.Min.Z;
    selector->SetActorLocation( v );
  }
}

// Returns to you the distance that you should set
// the camera away from tt to get a render on tt
// that has RadiusOnScreen
float AMyHUD::GetZDistance( float radiusWorldUnits, float radiusOnScreenPx, float texW, float fovyDegrees )
{
  // ZBack = radiusWorldUnits*screenWidthPX /
  //         (tan( fovy / 2 )*radiusOnScreenPX)
  float den = tanf( FMath::DegreesToRadians( fovyDegrees / 2.f ) ) * radiusOnScreenPx;
  return radiusWorldUnits*texW / den;
}

float AMyHUD::GetPxWidth( float radiusWorldUnits, float distanceToObject, float texW, float fovyDegrees )
{
  // radiusOnScreenPX = radiusWorldUnits*screenWidthPX / 
  //                    (tan( fovy / 2 )*distanceToObject)
  float den = tanf( FMath::DegreesToRadians( fovyDegrees / 2.f ) )*distanceToObject;
  return radiusWorldUnits*texW / den;
}

// Render onto tt (using renderer) sitting @ cameraPos,
// facing cameraDir, an object with radiusWorldUnits.
void AMyHUD::RenderScreen( USceneCaptureComponent2D* renderer,
  UTextureRenderTarget2D* tt, FVector objectPos, float radiusWorldUnits,
  FVector cameraDir )
{
  renderer->TextureTarget = tt;
  // http://stackoverflow.com/questions/3717226/
  // radiusOnScreenPX = radiusWorldUnits*SW/(tan(fov / 2) * Z);
  // ZBack = radiusWorldUnits*SW/(tan( fovy / 2 )*radiusOnScreenPX)
  // Calculate Z distance back for a given pixel radius
  // Set particular render properties & render the screen
  // to texture in w.
  float D = GetZDistance( radiusWorldUnits, tt->GetSurfaceWidth(),
    tt->GetSurfaceWidth(), renderer->FOVAngle );
  FVector renderPos = objectPos - cameraDir * D;
  renderer->SetRelativeLocationAndRotation( renderPos, cameraDir.Rotation().Quaternion() );
}

void AMyHUD::BeginDestroy()
{
  //UE_LOG( LogTemp, Warning, TEXT( "AMyHUD::BeginDestroy()" ) );
  if( ui ) {
    delete ui;
    ui = 0;
  }
  Super::BeginDestroy();  // PUT THIS LAST or the object may become invalid
}

