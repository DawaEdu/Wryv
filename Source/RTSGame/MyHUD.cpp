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
  ui->Name = "UI-root";
  
  // connect the mouse drag functions 
  ui->selectBox = new Border( FBox2DU(FVector2D(0,0),FVector2D(0,0)),8.f,FLinearColor::Green);

  // attach selectBox manipulation functions to ui's function handlers
  ui->OnClicked = [this]( FVector2D mouse ){
    ui->selectBox->Set( FBox2DU( mouse, mouse ) );
    return 0;
  };
  ui->OnDrag = [this]( FVector2D mouse ){
    FBox2DU box = ui->selectBox->Box;
    box.Max = mouse;
    ui->selectBox->Set( box );
    return 0;
  };

  ui->resources = new ResourcesWidget( 16, 4 );
  ui->Add( ui->resources );
  ui->resources->Name = "Resources";

  ui->rightPanel = new SidePanel( RightPanelTexture, PortraitTexture,
    MinimapTexture, FVector2D( 280, Canvas->SizeY ), FVector2D(4,4) );
  ui->rightPanel->Align = HotSpot::TopRight;
  ui->Add( ui->rightPanel );
  ui->rightPanel->Name = "Right side panel";

  // Attach functionality for minimap
  Minimap* minimap = ui->rightPanel->minimap;
  minimap->OnClicked = [minimap](FVector2D mouse){
    Game->flycam->SetCameraPosition( mouse / minimap->Size );
    return 0;
  };
  minimap->OnDrag = [minimap](FVector2D mouse){
    Game->flycam->SetCameraPosition( mouse / minimap->Size );
    return 0;
  };

  // Keep one of these for showing costs on flyover
  ui->costWidget = new CostWidget( TooltipBackgroundTexture, FVector2D(16,13), 8 );
  ui->Add( ui->costWidget );
  ui->costWidget->Align = HotSpot::CenterCenter;
  ui->costWidget->Name = "Cost widget";
  ui->costWidget->hidden = 1;

  ui->tooltip = new Tooltip( TooltipBackgroundTexture, FString("tooltip"), FVector2D(8,8) );
  ui->Add( ui->tooltip );
  ui->tooltip->hidden = 1;
  ui->tooltip->Name = "Tooltip";

  ui->buffs = new StackPanel( 0, FVector2D( 24, 24 ) );
  ui->buffs->Pad = FVector2D( 4, 4 );
  ui->Add( ui->buffs );
  ui->buffs->Name = "Buffs panel";

  ui->building = new StackPanel( 0, FVector2D( 50, 50 ) );
  ui->building->Pad = FVector2D( 4, 4 );
  ui->Add( ui->building );
  ui->building->Name = "Building panel";

  ui->statusBar = new StatusBar( FVector2D(Canvas->SizeX,Canvas->SizeY), 22, FLinearColor::Black );
  ui->Add( ui->statusBar );
  ui->statusBar->Name = "Status bar";
  HotSpot::TooltipWidget = ui->statusBar->Text; // Setup the default tooltip location

  // Create the panel for containing items/inventory
  ui->itemBelt = new SlotPalette( SlotPaletteTexture, 1, 4, FVector2D( 100,100 ), FVector2D( 8,8 ) );
  ui->Add( ui->itemBelt );
  ui->itemBelt->Align = HotSpot::BottomCenter;
  ui->itemBelt->Name = "Item belt";

  ui->controls = new Controls( PauseButtonTexture );
  ui->controls->pause->OnClicked = [this]( FVector2D mouse ){
    Game->pc->SetPause( !Game->pc->IsPaused() );  // pauses or unpauses the game
    if( Game->pc->IsPaused() )  ui->controls->pause->Icon = ResumeButtonTexture;
    else  ui->controls->pause->Icon = PauseButtonTexture;
    return 0;
  };
  ui->controls->Align = HotSpot::Top | HotSpot::ToLeftOfParent;
  ui->rightPanel->Add( ui->controls );

  // Map selection screen
  MapSelectionScreen *mss = ui->mapSelectionScreen = new MapSelectionScreen( ui->Size,
    TitleNameTexture,
    SolidWhiteTexture,
    MapSlotEntryBackgroundTexture,
    PortraitTexture,
    FVector2D( 120, 24 ), largeFont );

  ui->mapSelectionScreen->Align = HotSpot::CenterCenter;
  ui->Add( ui->mapSelectionScreen );
  ui->mapSelectionScreen->OKButton->OnClicked = [mss](FVector2D mouse){
    // OK button clicked, so load the map if there is a selected widget
    // else display error message
    if( mss->Selected )
      Game->flycam->LoadLevel( FName( *mss->Selected->GetText() ) );
    else
      Game->myhud->ui->statusBar->Set( "Select a map to load first" ) ;
    return 0;
  };


  StackPanel *p1 = new StackPanel( SolidWhiteTexture, FVector2D(16,16) );
  p1->Align = HotSpot::CenterLeft;
  p1->Pad = FVector2D( 12, 12 );
  p1->Margin = FVector2D( 40, 40 );
  p1->StackRight( new ImageWidget( GoldIconTexture ) );
  p1->StackRight( new ImageWidget( GoldIconTexture ) );
  p1->StackRight( new ImageWidget( GoldIconTexture ) );
  p1->StackRight( new ImageWidget( GoldIconTexture ) );
  p1->StackRight( new ImageWidget( GoldIconTexture ) );
  ui->Add( p1 );

  p1 = new StackPanel( SolidWhiteTexture, FVector2D(16,16) );
  p1->Align = HotSpot::CenterRight;
  p1->Pad = FVector2D( 12, 12 );
  p1->Margin = FVector2D( 40, 40 );
  p1->StackLeft( new ImageWidget( GoldIconTexture ) );
  p1->StackLeft( new ImageWidget( GoldIconTexture ) );
  p1->StackLeft( new ImageWidget( GoldIconTexture ) );
  p1->StackLeft( new ImageWidget( GoldIconTexture ) );
  p1->StackLeft( new ImageWidget( GoldIconTexture ) );
  ui->Add( p1 );

  p1 = new StackPanel( SolidWhiteTexture, FVector2D(16,16) );
  p1->Align = HotSpot::TopCenter;
  p1->Pad = FVector2D( 12, 12 );
  p1->Margin = FVector2D( 40, 40 );
  p1->StackBottom( new ImageWidget( GoldIconTexture ) );
  p1->StackBottom( new ImageWidget( GoldIconTexture ) );
  p1->StackBottom( new ImageWidget( GoldIconTexture ) );
  p1->StackBottom( new ImageWidget( GoldIconTexture ) );
  p1->StackBottom( new ImageWidget( GoldIconTexture ) );
  ui->Add( p1 );

  p1 = new StackPanel( SolidWhiteTexture, FVector2D(16,16) );
  p1->Align = HotSpot::BottomCenter;
  p1->Pad = FVector2D( 12, 12 );
  p1->Margin = FVector2D( 40, 40 );
  p1->StackTop( new ImageWidget( GoldIconTexture ) );
  p1->StackTop( new ImageWidget( GoldIconTexture ) );
  p1->StackTop( new ImageWidget( GoldIconTexture ) );
  p1->StackTop( new ImageWidget( GoldIconTexture ) );
  p1->StackTop( new ImageWidget( GoldIconTexture ) );
  ui->Add( p1 );


  /////
  // List the maps in the folder at the left side
  TArray<FAssetData> maps = ScanFolder( "/Game/Maps" );

  for( int i = 0; i < maps.Num(); i++ ) {
    ui->mapSelectionScreen->AddText( maps[i].AssetName.ToString(), HotSpot::CenterCenter );

    // Generate a thumbnail
    //UWorld* world = Cast<UWorld>( maps[i].GetAsset() );
    //if( world )
    //{
    //  UE_LOG( LogTemp, Warning, TEXT( "Loaded world map %s" ), *world->GetName() );
    //}
    //else
    //{
    //  UE_LOG( LogTemp, Warning, TEXT( "Could not load the world" ) );
    //}
    //static FAssetThumbnailPool pool( 5 );
    //FAssetThumbnail thumb( maps[i].GetAsset(), 256, 256, pool );
  }

  ui->missionObjectivesScreen = new MissionObjectivesScreen(
    MapSlotEntryBackgroundTexture, MapSlotEntryBackgroundTexture, 
    FVector2D( 300, 100 ), FVector2D( 8, 8 ) );
  ui->Add( ui->missionObjectivesScreen );

  // Add the mouseCursor last so that it renders last.
  ui->mouseCursor = new ImageWidget( MouseCursorHand.Texture );
  ui->Add( ui->mouseCursor );
  ui->mouseCursor->Name = "Mouse cursor";

  ui->Show( Game->gm->state );

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
    UE_LOG( LogTemp, Warning, TEXT("Entry %d:  Icon: %s, %d" ),
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
    ui->rightPanel->unitStats->Set( SelectedObject->PrintStats() );
    ui->rightPanel->portrait->Icon = SelectedObject->Widget.Icon;

    // 3. buffs. PER-FRAME: Clear & then re-draw the buffs
    ui->buffs->Clear(); // Clear any existing/previous buffs.
    if( AUnit* unit = Cast<AUnit>( SelectedObject ) )
    {
      // Go through the applied buffs
      for( int i = 0; i < unit->BonusTraits.size(); i++ )
      {
        Types buff = unit->BonusTraits[i].traits.Type;
        ui->buffs->StackRight( new ImageWidget( Game->myhud->widgets[ buff ].Icon ) );
      }
    }

    // 4. Clear & re-draw the spawn queue
    // Draw icons for the objects being spawned, and their progress.
    ui->building->Clear();
    SelectedObject->spawnQueue.clear();
    SelectedObject->spawnQueue.push_back( SpawningObject( 8.f, Types::UNITFOOTMAN ) );
    SelectedObject->spawnQueue.push_back( SpawningObject( 8.f, Types::UNITFOOTMAN ) );
    for( int i = 0; i < SelectedObject->spawnQueue.size(); i++ )
    {
      SpawningObject so = SelectedObject->spawnQueue[i];
      ImageWidget *img = new ImageWidget( Game->myhud->widgets[ so.Type ].Icon );
      img->Color.A = so.Percent();
      ui->building->StackRight(img);
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

  ui->resources->SetValues( FMath::RoundToInt( displayedGold ),
    FMath::RoundToInt( displayedLumber ), FMath::RoundToInt( displayedStone ) );
}

void AMyHUD::UpdateMouse()
{
  // Draw the mouse
  ui->mouseCursor->Margin = Game->flycam->getMousePos();
  // If the `NextSpell` is selected, cross hair is used, else hand.
  if( NextSpell ) {
    ui->mouseCursor->Icon = MouseCursorCrossHairs.Texture;
    ui->mouseCursor->hotpoint = MouseCursorCrossHairs.Hotpoint;
  }
  else {
    ui->mouseCursor->Icon = MouseCursorHand.Texture;
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

  // reflush the layout
  ui->layout( FVector2D( Canvas->SizeX, Canvas->SizeY ) );

  // Render the entire UI
  ui->render();

  // Render the cinematic from the material (must be setup to use
  // an Emissive channel)
  //DrawMaterial( MediaMaterial, 0, 0,
  //  Canvas->SizeX, Canvas->SizeY, 0, 0, 1, 1, 1.f );
}

// Detect clicks and mouse moves on the HUD
HotSpot* AMyHUD::MouseLeftDown( FVector2D mouse )
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::MouseLeftDown()") );
  return ui->Click( mouse, FVector2D(0,0) );
}

HotSpot* AMyHUD::MouseLeftUp( FVector2D mouse )
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::MouseLeftUp()") );
  return ui->Drop( mouse );
}

HotSpot* AMyHUD::MouseRightDown( FVector2D mouse )
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::MouseRightDown()") );
  return ui->RightClick( mouse, FVector2D(0,0) );
}

HotSpot* AMyHUD::MouseRightUp( FVector2D mouse )
{
  // Right up isn't handled.
  return 0;
}

HotSpot* AMyHUD::MouseMoved( FVector2D mouse )
{
  if( !Init )  return 0;

  // Drag events are when the left mouse button is down.
  if( Game->pc->IsDown( EKeys::LeftMouseButton ) )
  {
    return MouseDragged( mouse );
  }
  else
  {
    // Check against all UI buttons
    return MouseHovered( mouse );
  }

  return 0;
}

HotSpot* AMyHUD::MouseHovered( FVector2D mouse )
{
  // Check against all ui widgets that require hover.
  return ui->Hover( mouse );
}

HotSpot* AMyHUD::MouseDragged( FVector2D mouse )
{
  // Drag event.
  return ui->Drag( mouse );
}

HotSpot* AMyHUD::MouseDropped( FVector2D mouse )
{
  // Drop event.
  return ui->Drop( mouse );
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
  UE_LOG( LogTemp, Warning, TEXT( "AMyHUD::BeginDestroy()" ) );
  if( ui ) {
    delete ui;
    ui = 0;
  }
  Super::BeginDestroy();  // PUT THIS LAST or the object may become invalid
}

