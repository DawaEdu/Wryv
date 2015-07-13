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
#include "GroundPlane.h"
#include "Spell.h"
#include "GlobalFunctions.h"
#include "Widget.h"
#include "DrawDebugHelpers.h"

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

void AMyHUD::BeginDestroy()
{
  Super::BeginDestroy();
  if( ui ) {
    delete ui;
    ui = 0;
  }
}

void AMyHUD::InitWidgets()
{
  if( Init ) return;
  Init = 1;

  // Initialize the widgets that show the player gold, lumber, stone counts.
  ResourcesWidget::GoldTexture = GoldIconTexture;
  ResourcesWidget::LumberTexture = LumberIconTexture;
  ResourcesWidget::StoneTexture = StoneIconTexture;
  SolidWidget::SolidWhiteTexture = SolidWhiteTexture;
  SlotPalette::SlotPaletteTexture = SlotPaletteTexture;

  ui = new UserInterface(FVector2D(0,0), FVector2D(Canvas->SizeX, Canvas->SizeY));
  
  ui->resources = new ResourcesWidget( 16, 4, FVector2D(0,0) );
  ui->Add( ui->resources );

  ui->rightPanel = new Panel( RightPanelTexture, texMinimap, FVector2D( 256, Canvas->SizeY ), 4 );
  ui->Add( ui->rightPanel );

  ui->rightPanel->portrait->Icon = texIcon;
  // Attach functionality for minimap
  ui->rightPanel->minimap->OnClicked = [](FVector2D mouse){
    // get the click % and pan
    FVector2D perc = Game->myhud->ui->rightPanel->minimap->getHitPercent( mouse );
    Game->flycam->SetCameraPosition( perc );
  };

  // Keep one of these for showing costs on flyover
  ui->costWidget = new CostWidget( TooltipBackgroundTexture, FVector2D(16,13), 8 );
  ui->Add( ui->costWidget );
  ui->costWidget->Pos = FVector2D(300,300);

  ui->tooltip = new Tooltip( TooltipBackgroundTexture, FString("tooltip"), FVector2D(8,8) );
  ui->Add( ui->tooltip );
  //ui->tooltip->hidden = 1;
  ui->tooltip->Set( "HELLO" ) ;

  ui->buffs = new StackPanel( FVector2D( 24, 24 ), FVector2D( 4, 4 ) );
  ui->Add( ui->buffs );

  ui->building = new StackPanel( FVector2D( 50, 50 ), FVector2D( 4, 4 ) );
  ui->Add( ui->building );
  
  ui->statusBar = new StatusBar( FVector2D(Canvas->SizeX,Canvas->SizeY), 22, FLinearColor::Black );
  ui->Add( ui->statusBar );

  // Create the panel for containing items/inventory
  FVector2D size( 400, 100 );
  FVector2D pos( ( Canvas->SizeX - size.X )/2, Canvas->SizeY - size.Y - ui->statusBar->Size.Y );
  ui->itemBelt = new SlotPalette( SlotPaletteTexture, pos, size, 1, 4 );
  ui->Add( ui->itemBelt );

  ui->mouseCursor = new ImageWidget( MouseCursorHand.Texture, FVector2D(Canvas->SizeX/2,Canvas->SizeY/2) );
  ui->Add( ui->mouseCursor );

}

void AMyHUD::Setup()
{
  FVector v(0.f);
  FRotator r(0.f);
  if( !selector )  selector = GetWorld()->SpawnActor<AActor>( uClassSelector, v, r );
  if( !selectorAttackTarget )  selectorAttackTarget = GetWorld()->SpawnActor<AActor>( uClassSelectorA, v, r );
  if( !selectorShopPatron )  selectorShopPatron = GetWorld()->SpawnActor<AActor>( uClassSelectorShop, v, r );

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
      v.Z = box.Max.Z;
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
    RenderScreen( rendererIcon, texIcon, SelectedObject->Pos(), SelectedObject->GetBoundingRadius(), camDir );
    
    // 2. Text stats:
    // This is for the picture of the last clicked object. Generate a widget for the picture of the unit.
    // Print unit's stats into the stats panel
    ui->rightPanel->unitStats->SetText( SelectedObject->PrintStats() );
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

  ui->resources->SetValues( displayedGold, displayedLumber, displayedStone );
}

void AMyHUD::UpdateMouse()
{
  // Draw the mouse
  ui->mouseCursor->Pos = Game->flycam->getMousePos();
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
  
  // Render the minimap
  float boxMax = Game->flycam->floor->GetComponentsBoundingBox().GetExtent().GetMax();
  RenderScreen( rendererMinimap, texMinimap, Game->flycam->floor->Pos(), boxMax, FVector( 0, 0, -1 ) );
  
  // Buffs appear in lower center
  ui->layout( FVector2D( Canvas->SizeX, Canvas->SizeY ) );

  // Render the entire UI
  ui->render();
  
}

bool AMyHUD::Purchase( Types itemType )
{
  // The player acquires an itemType. Costs are looked up from the UnitsData table
  if( !Game->gm->playersTeam->CanAfford( itemType ) )  return 0;
  
  // Can afford it.
  Game->gm->playersTeam->Spend( itemType );
  // Increment inventory of this item on SelectedObject.

  // Purchase an item means
  if( IsUnit( itemType ) ) {
    // purchasing a unit adds it to your team
    if( SelectedObject )
    {
      // add to spawn queue of the lco
      SelectedObject->spawnQueue.push_back( SpawningObject( 0, itemType ) );
    }
  }
  else if( IsBuilding( itemType ) ) {
    // Pay for building and start to build it.
    NextBuilding = itemType;
  }
  else if( IsItem( itemType ) ) {
    // give item to SelectedObject
    if( SelectedObject )
    {
      AItemShop *shop = (AItemShop *)SelectedObject;
      if( shop && shop->patron )
      {
        shop->patron->Items.Push( itemType ) ;
      }
      else
      {
        UE_LOG( LogTemp, Warning, TEXT( "Not a shop or no patron" ) );
      }
    }
  }
  else if( IsSpell( itemType ) ) {
    
  }
  
  return 1;
}

void AMyHUD::RunEvent( Types buttonType )
{
  // Should be a widget that can do something
  if( IsItem( buttonType ) )
  {
    // Single use items
    UE_LOG( LogTemp, Warning, TEXT( "Effect %s" ), *Game->unitsData[ buttonType ].Name );
    // Apply the speedup to the SelectedObject (currently selected unit)
    ////SelectedObject->ApplyEffect( buttonType );
  }
  else if( IsSpell( buttonType ) )
  {
    // Single use spells. Spawn the spell object.
    UE_LOG( LogTemp, Warning, TEXT( "Casting spell %s" ), *Game->unitsData[ buttonType ].Name );
    // (some of which turn the mouse into a targetting cursor)
    // If the spell type requires a 2nd click, we set the nextSpell
    // variable so that the mouse cursor changes state and the spell
    // will be cast on next click.
    NextSpell = buttonType;
  }
  else if( IsBuilding( buttonType ) )
  {
    // We set this in the HUD. Once we return from this function,
    // the NextBuilding will be linked to the Peasant or whatever is selected
    NextBuilding = buttonType;
  }
}

// Detect clicks and mouse moves on the HUD
bool AMyHUD::MouseLeftDown( FVector2D mouse )
{
  if( !Init ) return 0;
  UE_LOG( LogTemp, Warning, TEXT("Mouse clicked on hud") );

  ui->itemBelt->Click( mouse );
  
  //UE_LOG( LogTemp, Warning, TEXT("didn't hit ui anywhere") );
  return 0;
}

bool AMyHUD::MouseLeftUp( FVector2D mouse )
{
  // The left mouse button was let go of, meaning the box selection may have ended
  // Form a box shaped selection
  TArray<AActor*> actors;
  selectBox += mouse;
  //Game->myhud->GetActorsInSelectionRectangle( selectBox.Min, selectBox.Max, actors );
  //for( int i = 0; i < actors.Num(); i++ )
  //{
  //  UE_LOG( LogTemp, Warning, TEXT("Selected %s"), *actors[i]->GetName() );
  //}
  return 1;
}

bool AMyHUD::MouseRightDown( FVector2D mouse )
{
  return 1;
}

bool AMyHUD::MouseRightUp( FVector2D mouse )
{
  return 1;
}

bool AMyHUD::MouseDragged( FVector2D mouse )
{
  // Drag event. You cannot render here, Canvas is only initialized in DrawHUD()
  // capture minimap event first
  if( ui->rightPanel->minimap->hit( mouse ) )
  {
    // get the click % and pan
    FVector2D perc = ui->rightPanel->minimap->getHitPercent( mouse );
    Game->flycam->SetCameraPosition( perc );
    return 1;
  }
  return 0;
}

bool AMyHUD::MouseHovered( FVector2D mouse )
{
  // Check against all ui widgets that require hover.
  ui->itemBelt->Hover( mouse );
  return 1;
}

bool AMyHUD::MouseMoved( FVector2D mouse )
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

void AMyHUD::Tick( float t )
{
  if( selector   &&   SelectedObject )
  {
    FVector v = SelectedObject->Pos();
    FBox box = Game->flycam->floor->GetComponentsBoundingBox();
    v.Z = box.Max.Z;
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


