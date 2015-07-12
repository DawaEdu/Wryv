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

float AMyHUD::BarSize = 22.f;
float AMyHUD::Margin = 4.f;

AMyHUD::AMyHUD(const FObjectInitializer& PCIP) : Super(PCIP)
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::AMyHUD(ctor)") );
  selector = 0;
  selectorAttackTarget = 0;
  selectorShopPatron = 0;
  lastClickedObject = 0;
  hoverWidget = 0;
  itemSlots = 0;
  buffs = 0;
  spawnQueue = 0;

  Init = 0; // Have the slots & widgets been initialied yet? can only happen
  // in first call to draw.
}

void AMyHUD::BeginDestroy()
{
  Super::BeginDestroy();

  delete rightPanel;
  delete resourcesWidget;
  delete tooltip;
  delete itemSlots;
  delete buffs;
  delete spawnQueue;
}

void AMyHUD::InitWidgets()
{
  if( Init ) return;
  Init=1;

  // Initialize the widgets that show the player gold, lumber, stone counts.
  ResourcesWidget::GoldTexture = GoldIconTexture;
  ResourcesWidget::LumberTexture = LumberIconTexture;
  ResourcesWidget::StoneTexture = StoneIconTexture;
  resourcesWidget = new ResourcesWidget( 16 );
  tooltip = new Tooltip( TooltipBackgroundTexture, FString("tooltip"), FVector2D(8,8) );

  // Create the panel for containing items/inventory
  FVector2D size( 400, 100 );
  FVector2D pos( ( Canvas->SizeX - size.X )/2, Canvas->SizeY - size.Y );
  itemSlots = new SlotPalette( 1, 4, SlotPanelTexture, pos, size );
  //AddDebugText( "Hello", this, 25.f, FVector(0.f), FVector(0.f), FLinearColor::White );
  
  minimap = new ImageWidget( texMinimap, FVector2D(0,0),
    FVector2D( texMinimap->GetSurfaceWidth(), texMinimap->GetSurfaceHeight() ) );

  rightPanel = new Panel( RightPanelTexture, FVector2D(256, Canvas->SizeY), 4 );
  rightPanel->Icon->Icon = texIcon;

  buffs = new StackPanel( FVector2D( 16, 16 ), FVector2D( 4, 4 ) );

  spawnQueue = new StackPanel( FVector2D( 64, 64 ), FVector2D( 6, 6 ) );

  // Keep one of these for showing costs on flyover
  costWidget = new CostWidget();

  mouseCursor = new ImageWidget( MouseCursorHand.Texture, FVector2D(Canvas->SizeX/2,Canvas->SizeY/2) );
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
    UE_LOG( LogTemp, Warning, TEXT("%d Icon: %s, %d pointer=%p" ),
      j, *p.second.Label, (int)p.second.Type.GetValue(), p.second.Icon );
    j++;
  }

  // check if the canvas width is available
  // It doesn't look like these sizes are correct on initial load,
  // because the canvas may not be setup yet, but we put it here anyway
  float w = 256.f, h = 256.f, panelWidth = 256.f;
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::SetupPlayerInputComponent() w=%f, h=%f"), w, h );
  
  // No selected building to begin with
  lastClickedObject = 0;
  
  rendererIcon = GetComponentByName<USceneCaptureComponent2D>( this, "rendererIcon" ); //rs[0];
  rendererMinimap = GetComponentByName<USceneCaptureComponent2D>( this, "rendererMinimap" ); //rs[1];
}

void AMyHUD::DrawWidget( FWidgetData& w )
{
  UE_LOG( LogTemp, Warning, TEXT( "can't render widgetdata's!" ) );

  if( w.Icon )
  {
    //DrawTexture( w.Icon, w.Pos.X, w.Pos.Y, w.Size.X, w.Size.Y, 0, 0, 1, 1 );
  }
  else
  {
    UE_LOG( LogTemp, Warning, TEXT( "No texture in %s %s" ), *GetEnumName( w.Type ), *w.Label );
  }

  //DrawText( w.Label, w.TextColor, w.TextPos.X, w.TextPos.Y );
}

void AMyHUD::DrawGroup( TArray< TEnumAsByte<Types> >& v, float xPos, float yPos, float size, float margin, bool horizontalStack, bool label, bool purchaseables )
{
  // Collect as a map. How many of each type do you have?
  map< Types, int > items;
  for( int i = 0; i < v.Num(); i++ )
    items[ v[i] ]++;

  // Draw Name:Qty of each item
  int i = 0;
  for( pair<Types,int> p : items )
  {
    // take stock copies of the building widgets and draw it
    FWidgetData bw = widgets[ p.first ];
    if( label ) // Draw the QTY
      bw.Label = GetEnumName( p.first ) + FString::Printf( TEXT(" x %d"), p.second ) ;
    
    bw.Size = FVector2D( size, size );

    int cols = 3;
    // Stacking horizontal/vertical
    ////bw.Pos = FVector2D( xPos, yPos );
    ////bw.Pos.X += (i%cols)*(bw.Size.X + margin);
    ////bw.Pos.Y -= (i/cols)*(bw.Size.Y + margin);
    ////
    ////bw.TextPos = FVector2D( bw.Pos.X, bw.Pos.Y + bw.Size.Y );

    //UE_LOG( LogTemp, Warning, TEXT("i=%d"), i );

    // To track hits, we store this in the frameWidgets collection
    frameWidgets.push_back( bw );
    DrawWidget( bw );

    i++;
  }
}

void AMyHUD::SetAttackTargetSelector( AGameObject* target )
{
  if( target )
  {
    FVector v = lastClickedObject->attackTarget->Pos();
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
    if( AItemShop *is = Cast<AItemShop>( lastClickedObject ) )
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

void AMyHUD::DrawSidebar()
{
  float w = Canvas->SizeX;
  float h = Canvas->SizeY;

  rightPanel->Pos.X = w - rightPanel->Size.X;
  rightPanel->render();

  // Minimap is lower left
  minimap->Pos.X = 0;
  minimap->Pos.Y = h - minimap->Size.Y - BarSize - 0;
  
  // Draw the minimap
  FBox box = Game->flycam->floor->GetComponentsBoundingBox();
  float boxMax = box.GetExtent().GetMax();

  // Render the minimap
  RenderScreen( rendererMinimap, texMinimap, Game->flycam->floor->Pos(), boxMax, FVector( 0, 0, -1 ) );
  minimap->render();
  
  if( lastClickedObject )
  {
    // render last-clicked object to texture
    // zoom back by radius of bounding sphere of clicked object
    FVector camDir( .5f, .5f, -FMath::Sqrt( 2.f ) );
    RenderScreen( rendererIcon, texIcon, lastClickedObject->Pos(), lastClickedObject->GetBoundingRadius(), camDir );
    
    // This is for the picture of the last clicked object. Generate a widget for the picture of the unit.
    // Print unit's stats into the stats panel
    rightPanel->UnitStats->SetText( lastClickedObject->PrintStats() );
    //rightPanel->icon->Icon = lastClickedObject->Widget.Icon;

    // Draw the buffs
    buffs->Clear(); // Clear any existing/previous buffs.
    if( AUnit* unit = Cast<AUnit>( lastClickedObject ) )
    {
      // Go through the applied buffs
      for( int i = 0; i < unit->BonusTraits.size(); i++ )
      {
        Types buff = unit->BonusTraits[i].traits.Type;
        buffs->StackRight( new ImageWidget( Game->myhud->widgets[ buff ].Icon ) );
      }
    }

    ////////////////////
    // Draw icons for the objects being spawned, and their progress.
    for( int i = 0; i < lastClickedObject->spawnQueue.size(); i++ )
    {
      Types type = lastClickedObject->spawnQueue[i].type;
      FWidgetData w = Game->myhud->widgets[ type ];
      w.Size = FVector2D( 100.f, 100.f );
      // Draw a cooldownpie
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
}

void AMyHUD::DrawTopBar()
{
  //DrawRect( FLinearColor::Black, 0, 0, Canvas->SizeX, BarSize );
  resourcesWidget->gold->SetText( FString::Printf( TEXT("%.0f"), displayedGold ) );
  resourcesWidget->lumber->SetText( FString::Printf( TEXT("%.0f"), displayedLumber ) );
  resourcesWidget->stone->SetText( FString::Printf( TEXT("%.0f"), displayedStone ) );
  resourcesWidget->render();
}

void AMyHUD::DrawBottomBar()
{
  // Bottom bar
  DrawRect( FLinearColor::Black, 0, Canvas->SizeY - BarSize, Canvas->SizeX, BarSize );
  DrawText( statusMsg, FLinearColor::White, 0, Canvas->SizeY - BarSize );
}

void AMyHUD::DrawMouseCursor()
{
  // Draw the mouse
  mouseCursor->Pos = Game->flycam->getMousePos();

  // If the `NextSpell` is selected, cross hair is used, else hand.
  if( NextSpell ) {
    mouseCursor->Icon = MouseCursorCrossHairs.Texture;
    mouseCursor->hotpoint = MouseCursorCrossHairs.Hotpoint;
  }
  else {
    mouseCursor->Icon = MouseCursorHand.Texture;
    mouseCursor->hotpoint = MouseCursorHand.Hotpoint;
  }

  mouseCursor->render();
}

void AMyHUD::DrawHUD()
{
  // Canvas is only initialized here.
  Super::DrawHUD();
  HotSpot::hud = this;
  InitWidgets();

  UpdateDisplayedResources();
  DrawBottomBar();
  DrawTopBar();
  
  itemSlots->render();

  DrawSidebar();
  DrawMouseCursor();
}

bool AMyHUD::Purchase( Types itemType )
{
  // The player acquires an itemType. Costs are looked up from the UnitsData table
  if( !Game->gm->playersTeam->CanAfford( itemType ) )  return 0;
  
  // Can afford it.
  Game->gm->playersTeam->Spend( itemType );
  // Increment inventory of this item on lastClickedObject.

  // Purchase an item means
  if( IsUnit( itemType ) ) {
    // purchasing a unit adds it to your team
    if( lastClickedObject )
    {
      // add to spawn queue of the lco
      lastClickedObject->spawnQueue.push_back( SpawningObject( 0, itemType ) );
    }
  }
  else if( IsBuilding( itemType ) ) {
    // Pay for building and start to build it.
    NextBuilding = itemType;
  }
  else if( IsItem( itemType ) ) {
    // give item to lastClickedObject
    if( lastClickedObject )
    {
      AItemShop *shop = (AItemShop *)lastClickedObject;
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
    // Apply the speedup to the lastClickedObject (currently selected unit)
    ////lastClickedObject->ApplyEffect( buttonType );
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

  itemSlots->Click( mouse );
  
  // capture minimap event first
  if( minimap->hit( mouse ) )
  {
    // get the click % and pan
    FVector2D perc = minimap->getHitPercent( mouse );
    Game->flycam->SetCameraPosition( perc );
    return 1;
  }

  for( int i = 0 ; i < frameWidgets.size(); i++ )
  {
    FWidgetData &w = frameWidgets[i];

    // if one of the buildings 
    //if( w.hit( mouse ) )
    //{
    //  UE_LOG( LogTemp, Warning, TEXT("hit %s"), *GetEnumName( w.Type ) );
    //  // since widgets map only populated at program begin (not between game frames)
    //  Purchase( w.Type );
    //  //RunEvent( w.Type );
    //  return 1; /// a building widget was hit
    //}
    //else
    //{
    //  UE_LOG( LogTemp, Warning, TEXT("missed widget %s"), *GetEnumName( w.Type ) );
    //}
  }

  //UE_LOG( LogTemp, Warning, TEXT("didn't hit ui anywhere") );
  return 0;
}

bool AMyHUD::MouseLeftUp( FVector2D mouse )
{
  // The left mouse button was let go of, meaning the box selection may have ended
  // Form a box shaped selection
  TArray<AActor*> actors;
  selectBox += mouse;
  Game->myhud->GetActorsInSelectionRectangle( selectBox.Min, selectBox.Max, actors );

  for( int i = 0; i < actors.Num(); i++ )
  {
    UE_LOG( LogTemp, Warning, TEXT("Selected %s"), *actors[i]->GetName() );
  }
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
  if( minimap->hit( mouse ) )
  {
    // get the click % and pan
    FVector2D perc = minimap->getHitPercent( mouse );
    Game->flycam->SetCameraPosition( perc );
    return 1;
  }
  return 0;
}

bool AMyHUD::MouseHovered( FVector2D mouse )
{
  // Check against the slotPanel
  itemSlots->Hover( mouse );
  hoverWidget = 0;
  for( int i = 0 ; i < frameWidgets.size(); i++ )
  {
    FWidgetData &w = frameWidgets[i];

    // Update the status message with the status info
    // of the item UNDER the mouse cursor
    //if( w.hit( mouse ) )
    //{
    //  hoverWidget = &w; // save hoverWidget reference for this frame.
    //  return 1;
    //}
  }
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
  if( selector   &&   lastClickedObject )
  {
    FVector v = lastClickedObject->Pos();
    FBox box = Game->flycam->floor->GetComponentsBoundingBox();
    v.Z = box.Max.Z;
    selector->SetActorLocation( v );
  }

  frameWidgets.clear();
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

