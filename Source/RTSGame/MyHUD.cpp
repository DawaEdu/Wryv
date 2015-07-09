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

float AMyHUD::BarSize = 22.f;
float AMyHUD::Margin = 4.f;
FVector2D AMyHUD::IconSize( 64.f, 64.f );

AMyHUD::AMyHUD(const FObjectInitializer& PCIP) : Super(PCIP)
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::AMyHUD(ctor)") );
  selector = 0;
  selectorAttackTarget = 0;
  selectorShopPatron = 0;
  lastClickedObject = 0;
  hoverWidget = 0;
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
  float den =
    tanf( FMath::DegreesToRadians( fovyDegrees / 2.f ) )*distanceToObject;
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

void AMyHUD::Setup()
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::Setup()") );
  FVector v(0.f);
  FRotator r(0.f);
  if( !selector )
    selector = GetWorld()->SpawnActor<AActor>( uClassSelector, v, r );
  if( !selectorAttackTarget )
    selectorAttackTarget = GetWorld()->SpawnActor<AActor>( uClassSelectorA, v, r );
  if( !selectorShopPatron )
    selectorShopPatron = GetWorld()->SpawnActor<AActor>( uClassSelectorShop, v, r );
  Game->Init();
  UE_LOG( LogTemp, Warning, TEXT( "PROCESSING DATATABLE (%d members)" ), Game->unitsData.size() );
  
  // Connect Widgets & UnitsData with mappings from Types
  vector< Types > types;
  for( int i = 0; i < Types::MAX; i++ )
  {
    types.push_back( (Types)i );
  }
  
  // Pull all the UCLASS names from the Blueprints we created.
  for( int i = 0; i < UnitTypeUClasses.Num(); i++ )
  {
    Types ut = UnitTypeUClasses[i].Type;
    UClass* uc = UnitTypeUClasses[i].uClass;

    if( !uc ) continue;
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
    widgets[ ut ] = unit->Widget; // 
    
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
  float w = 256.f;
  float h = 256.f;
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::SetupPlayerInputComponent() w=%f, h=%f"), w, h );
  float panelWidth = 256.f;
  
  // No selected building to begin with
  lastClickedObject = 0;
  
  float m = 4.f;
  widgetGold.Pos       = FVector2D( 0, m );
  widgetGold.TextPos   = FVector2D( 16, m );
  widgetLumber.Pos     = FVector2D( 100, m );
  widgetLumber.TextPos = FVector2D( 120, m );
  widgetStone.Pos      = FVector2D( 200, m );
  widgetStone.TextPos  = FVector2D( 220, m );
  widgetIcon.Pos       = FVector2D( 200, 200 );
  widgetIcon.Icon      = texIcon;
  widgetIcon.Size      = FVector2D( texIcon->GetSurfaceWidth(), texIcon->GetSurfaceHeight() ) ;
  widgetMinimap.Pos    = FVector2D( m, m );
  widgetMinimap.Icon   = texMinimap;
  widgetMinimap.Size   = FVector2D( texMinimap->GetSurfaceWidth(), texMinimap->GetSurfaceHeight() );

  rendererIcon = GetComponentByName<USceneCaptureComponent2D>( this, "rendererIcon" ); //rs[0];
  rendererMinimap = GetComponentByName<USceneCaptureComponent2D>( this, "rendererMinimap" ); //rs[1];
}

// returns the advancement in X 
float AMyHUD::DrawCost( Types res, float x, float size, int cost )
{
  static map< Types, FWidgetData > widgets = {
    { Types::RESGOLDMINE, widgetGold },
    { Types::RESTREE, widgetLumber },
    { Types::RESSTONE, widgetStone },
  }; 
  
  FWidgetData w = widgets[res];
  w.Pos = FVector2D( x, Canvas->SizeY - BarSize );
  w.Size = FVector2D( size, size );
  w.Label = FString::Printf( TEXT( "%d" ), cost );
  w.TextPos = w.Pos + FVector2D( w.Size.X, 0 );
  float ow, oh;
  GetTextSize( w.Label, ow, oh );
  DrawWidget( w );
  return w.Size.X + ow;
}

void AMyHUD::DrawWidget( FWidgetData& w )
{
  if( w.Icon )
  {
    DrawTexture( w.Icon, w.Pos.X, w.Pos.Y, w.Size.X, w.Size.Y, 0, 0, 1, 1 );
  }
  else
  {
    UE_LOG( LogTemp, Warning, TEXT( "No texture in %s %s" ), *GetEnumName( w.Type ), *w.Label );
  }

  DrawText( w.Label, w.TextColor, w.TextPos.X, w.TextPos.Y );
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
    bw.Pos = FVector2D( xPos, yPos );
    bw.Pos.X += (i%cols)*(bw.Size.X + margin);
    bw.Pos.Y -= (i/cols)*(bw.Size.Y + margin);

    bw.TextPos = FVector2D( bw.Pos.X, bw.Pos.Y + bw.Size.Y );
    bw.isPurchaseButton = purchaseables;

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

  // width of the panel minus margin WOOD Background
  float m = 4.f;
  widgetBkg.Size.X = 256 + 2*m;
  widgetBkg.Size.Y = h;
  widgetBkg.Pos.X = w - widgetBkg.Size.X;
  widgetBkg.Pos.Y = 0.f;
  DrawWidget( widgetBkg );

  // widgetIcon is top right
  widgetIcon.Pos.X = w - widgetIcon.Size.X - m;
  widgetIcon.Pos.Y = BarSize + m;
  // Minimap is lower left
  widgetMinimap.Pos.X = m;
  widgetMinimap.Pos.Y = h - widgetMinimap.Size.Y - BarSize - m;
  
  // Draw the minimap
  FBox box = Game->flycam->floor->GetComponentsBoundingBox();
  float boxMax = box.GetExtent().GetMax();
  RenderScreen( rendererMinimap, texMinimap,
    Game->flycam->floor->Pos(), boxMax, FVector( 0, 0, -1 ) );
  DrawWidget( widgetMinimap );

  float yPos = BarSize; // start at the beginning of the top bar
  if( lastClickedObject )
  {
    SetAttackTargetSelector( lastClickedObject->attackTarget );
    
    // render last-clicked object to texture
    // zoom back by radius of bounding sphere of clicked object
    float objRadius, hh;
    lastClickedObject->GetComponentsBoundingCylinder( objRadius, hh, 1 );
    objRadius = FMath::Max( objRadius, hh );
    FVector camDir( .5f, .5f, -FMath::Sqrt( 2.f ) );
    
    RenderScreen( rendererIcon, texIcon, lastClickedObject->Pos(), objRadius, camDir );
    // This is for the picture of the last clicked object. Generate a widget for the picture of the unit.
    DrawWidget( widgetIcon );

    // Draw the spawnables
    // Check the type of what was clicked
    FString fs = Game->unitsData[ lastClickedObject->UnitsData.Type ].Name;
    switch( lastClickedObject->UnitsData.Type )
    {
      case Types::RESGOLDMINE:  case Types::RESSTONE:  case Types::RESTREE:
        {
          AResource* res = Cast<AResource>( lastClickedObject );
          fs += FString::Printf( TEXT(" [%d]"), res->Amount );
        }
        break;
      case Types::BLDGFARM:
        {
          fs += FString::Printf( TEXT(" Supply: [%d]"), lastClickedObject->UnitsData.FoodProvided );
        }
        break;
      default:  break;
    }

    float x = widgetBkg.left() + BarSize;
    yPos = widgetBkg.Size.X;
    
    // Draw the name of object, label position
    DrawText( fs, FLinearColor::White, x, yPos );
    //UE_LOG( LogTemp, Warning, TEXT("%s spawns: "), *lastClickedObject->UnitsData.Name );
    
    // Draw his special attacks.
    if( AUnit* unit = Cast<AUnit>( lastClickedObject ) )
    {
      if( unit->Spells.Num() )  // DRAW SPELLS
      {
        DrawText( TEXT("Spells"), FLinearColor::White, x, yPos += BarSize);
        DrawGroup( unit->Spells, x, yPos += BarSize, IconSize.X, BarSize, 1, 1, 0 );
      }

      yPos += IconSize.X + BarSize;
      if( unit->Items.Num() )  // DRAW ITEMS
      {
        DrawGroup( unit->Items, 20, Canvas->SizeY - IconSize.Y - 2*BarSize, IconSize.X, BarSize, 1, 1, 0 );
      }

      TArray< TEnumAsByte< Types > > buffs;
      
      // Go through the applied buffs
      for( int i = 0; i < unit->BonusTraits.size(); i++ )
        buffs.Push( unit->BonusTraits[i].traits.Type );
      if( buffs.Num() )  // DRAW ALL EFFECTIVE BUFFS
      {
        // vertical stack
        DrawGroup( buffs, 20, Canvas->SizeY - 2*IconSize.Y - 3*BarSize, IconSize.X/2.f, BarSize, 0, 0, 0 );
      }
    }

    yPos += IconSize.Y + BarSize;
    if( lastClickedObject->UnitsData.Spawns.Num() ) // DRAW SPAWNS (with costs)
    {
      DrawText( TEXT("Spawns"), FLinearColor::White, x, yPos );
      DrawGroup( lastClickedObject->UnitsData.Spawns, x, yPos += BarSize, IconSize.X, BarSize, 1, 1, 1 );
      // go thru all the unitsdata spawns and 
    }

    ////////////////////
    // Draw the objects being spawned
    for( int i = 0; i < lastClickedObject->spawnQueue.size(); i++ )
    {
      AGameObject *g = lastClickedObject->spawnQueue[i];
      FWidgetData w = g->Widget;
      w.Size = FVector2D( 100.f, 100.f );
      w.Pos = FVector2D( i*w.Size.X, 25 );
      DrawWidget( w );
    }
  }
}

void AMyHUD::DrawTooltipHover()
{
  if( !hoverWidget )  return;

  FUnitsDataRow &data = Game->unitsData[ hoverWidget->Type ];
  if( hoverWidget->isPurchaseButton ) {
    statusMsg = DrawToolTipBuyCost( data );
  }
  else {
    statusMsg = DrawToolTipUseCost( data );
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

void AMyHUD::DrawInfoBars()
{
  UpdateDisplayedResources();

  float w = Canvas->SizeX;
  float h = Canvas->SizeY;
  
  DrawRect( FLinearColor::Black, 0, 0, w, BarSize );
  
  widgetGold.Label = FString::Printf( TEXT("Gold %d"), Game->gm->playersTeam->Gold );
  widgetLumber.Label = FString::Printf( TEXT("Lumber %d"), Game->gm->playersTeam->Lumber );
  widgetStone.Label = FString::Printf( TEXT("Stone %d"), Game->gm->playersTeam->Stone );

  // Topbar
  DrawWidget( widgetGold );
  DrawWidget( widgetLumber );
  DrawWidget( widgetStone );

  // Bottom bar
  DrawRect( FLinearColor::Black, 0, h - BarSize, w, BarSize );
  DrawText( statusMsg, FLinearColor::White, 0, h - BarSize );

  
}

void AMyHUD::DrawMouseCursor()
{
  // Draw the mouse
  widgetMouseCursorCrossHairs.Pos = widgetMouseCursorHand.Pos = Game->flycam->getMousePos();
  // If the `NextSpell` is selected, cross hair is used, else hand.
  if( NextSpell ) {
    widgetMouseCursorCrossHairs.Pos -= widgetMouseCursorCrossHairs.Size/2.f;
    DrawWidget( widgetMouseCursorCrossHairs );
  }
  else {
    DrawWidget( widgetMouseCursorHand );
  }
}

void AMyHUD::DrawHUD()
{
  // Canvas is only initialized here.
  Super::DrawHUD();
  DrawInfoBars();
  DrawSidebar();
  DrawTooltipHover();

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
      lastClickedObject->spawnQueue.push_back( Game->Make( itemType, FVector(), lastClickedObject->team->teamId ) );
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
    lastClickedObject->ApplyEffect( buttonType );
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
bool AMyHUD::MouseClicked( FVector2D mouse )
{
  UE_LOG( LogTemp, Warning, TEXT("mouse clicked on hud") );

  // capture minimap event first
  if( widgetMinimap.hit( mouse ) )
  {
    // get the click % and pan
    FVector2D perc = widgetMinimap.getHitPercent( mouse );
    Game->flycam->SetCameraPosition( perc );
    return 1;
  }

  for( int i = 0 ; i < frameWidgets.size(); i++ )
  {
    FWidgetData &w = frameWidgets[i];

    // if one of the buildings 
    if( w.hit( mouse ) )
    {
      UE_LOG( LogTemp, Warning, TEXT("hit %s"), *GetEnumName( w.Type ) );
      // since widgets map only populated at program begin (not between game frames)
      if( w.isPurchaseButton )
        Purchase( w.Type );
      else
        RunEvent( w.Type );
      return 1; /// a building widget was hit
    }
    else
    {
      UE_LOG( LogTemp, Warning, TEXT("missed widget %s"), *GetEnumName( w.Type ) );
    }
  }

  //UE_LOG( LogTemp, Warning, TEXT("didn't hit ui anywhere") );
  return 0;
}

bool AMyHUD::MouseMoved( FVector2D mouse )
{
  // You cannot render here, Canvas is only initialized in DrawHUD()
  // capture minimap event first
  if( widgetMinimap.hit( mouse ) && Game->pc->GetInputKeyTimeDown( EKeys::LeftMouseButton ) )
  {
    // get the click % and pan
    FVector2D perc = widgetMinimap.getHitPercent( mouse );
    Game->flycam->SetCameraPosition( perc );
    return 1;
  }

  hoverWidget = 0;
  for( int i = 0 ; i < frameWidgets.size(); i++ )
  {
    FWidgetData &w = frameWidgets[i];

    // Update the status message with the status info
    // of the item UNDER the mouse cursor
    if( w.hit( mouse ) )
    {
      hoverWidget = &w; // save hoverWidget reference for this frame.
      return 1;
    }
  }

  return 0;
}

FString AMyHUD::DrawToolTipBuyCost( FUnitsDataRow& ud )
{
  FString tt = FString::Printf( TEXT("%s"), *ud.Name );
  float ow, oh;
  GetTextSize( tt, ow, oh );
  // draw the price
  ow += DrawCost( Types::RESGOLDMINE, ow, BarSize, ud.GoldCost );
  ow += DrawCost( Types::RESTREE, ow, BarSize, ud.LumberCost );
  ow += DrawCost( Types::RESSTONE, ow, BarSize, ud.StoneCost );
  return tt;
}

FString AMyHUD::DrawToolTipUseCost( FUnitsDataRow& ud )
{
  FString tt = FString::Printf( TEXT("%s - %s "), *ud.Name, *ud.Description );
  float ow, oh;
  GetTextSize( tt, ow, oh );
  // draw the price
  if( ud.ManaCost )
  ow += DrawCost( Types::RESGOLDMINE, ow, BarSize, ud.GoldCost );
  ow += DrawCost( Types::RESTREE, ow, BarSize, ud.LumberCost );
  ow += DrawCost( Types::RESSTONE, ow, BarSize, ud.StoneCost );
  return tt;
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


