#include "RTSGame.h"
#include "FlyCam.h"
#include "Resource.h"
#include "PlayerControl.h"
#include "MyHUD.h"
#include "Unit.h"
#include "UnitsData.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "GroundPlane.h"
#include "Spell.h"

float AMyHUD::margin = 22.f;
FVector2D AMyHUD::IconSize( 64.f, 64.f );

AMyHUD::AMyHUD(const FObjectInitializer& PCIP) : Super(PCIP)
{
  UE_LOG( LogTemp, Warning, TEXT("AMyHUD::AMyHUD(ctor)") );
  selector = 0;
  selectorAttackTarget = 0;
  lastClickedObject = 0;
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

    UE_LOG( LogTemp, Warning, TEXT("Loaded unit=%s / UClass=%s" ),
      *GetEnumName( ut ), *uc->GetClass()->GetName() );
    if( !uc ) continue;

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
}

void AMyHUD::DrawWidget( FWidgetData& w )
{
  if( !w.Type )
  {
    UE_LOG( LogTemp, Warning, TEXT( "No texture" ) );
  }
  if( w.Icon )
  {
    DrawTexture( w.Icon, w.Pos.X, w.Pos.Y, w.Size.X, w.Size.Y, 0, 0, 1, 1 );
  }
  else
  {
    UE_LOG( LogTemp, Warning, TEXT( "No texture in %s %s" ),
      *GetEnumName( w.Type ), *w.Label );
  }
  DrawText( w.Label, w.TextColor, w.TextPos.X, w.TextPos.Y );
}

void AMyHUD::DrawGroup( TArray< TEnumAsByte<Types> >& v, float xPos, float yPos, float size, float margin, bool horizontalStack, bool label )
{
  // Collect as a map
  map< Types, int > items;
  for( int i = 0; i < v.Num(); i++ )
    items[ v[i] ]++;

  int i = 0;
  // Draw all the icons that this unit has available for it.
  for( pair<Types,int> p : items )
  {
    // take stock copies of the building widgets and draw it
    FWidgetData bw = widgets[ p.first ];
    if( label ) bw.Label = GetEnumName( p.first ) + FString::Printf( TEXT(" x %d"), p.second ) ;
    else bw.Label = "";
    bw.Size = FVector2D( size, size );
    // Stacking horizontal/vertical
    bw.Pos = FVector2D( xPos + i*(bw.Size.X + margin)*horizontalStack,
      yPos - i*(bw.Size.Y + margin)*(!horizontalStack) );
    bw.TextPos = FVector2D( bw.Pos.X, bw.Pos.Y + bw.Size.Y );
    
    // To track hits, we store this in the frameWidgets collection
    frameWidgets.push_back( bw );
    DrawWidget( bw );

    i++;
  }
}

void AMyHUD::DrawSidebar()
{
  float w = Canvas->SizeX;
  float h = Canvas->SizeY;

  // width of the panel minus margin WOOD Background
  widgetBkg.Size.X = 256.f;
  widgetBkg.Size.Y = h;
  widgetBkg.Pos.X = w - widgetBkg.Size.X;
  widgetBkg.Pos.Y = 0.f;
  DrawWidget( widgetBkg );

  float yPos = margin;
  if( lastClickedObject )
  {
    if( lastClickedObject->attackTarget )
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

    // This is for the picture of the last clicked object
    float x = w - widgetBkg.Size.X + 1*margin;
    float size = widgetBkg.Size.X - 2*margin;
    
    // Generate a widget for the picture of the unit
    lastClickedObject->Widget.Pos = FVector2D( x, 2*margin );
    lastClickedObject->Widget.Size = FVector2D( size, size );
    DrawWidget( lastClickedObject->Widget );

    // Draw the spawnables
    // Check the type of what was clicked
    FString fs = Game->unitsData[ lastClickedObject->UnitsData.Type ].Name;
    switch( lastClickedObject->UnitsData.Type )
    {
      case Types::RESGOLDMINE:
      case Types::RESSTONE:
      case Types::RESTREEDECIDUOUS:
      case Types::RESTREEEVERGREEN:
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

    x = widgetBkg.left() + margin;
    yPos = widgetBkg.Size.X;
    size = 64.f;

    // Draw the name of object, label position
    DrawText( fs, FLinearColor::White, x, yPos );
    //UE_LOG( LogTemp, Warning, TEXT("%s spawns: "), *lastClickedObject->UnitsData.Name );
    
    // Draw his special attacks.
    if( AUnit* unit = Cast<AUnit>( lastClickedObject ) )
    {
      if( unit->Spells.Num() )
      {
        DrawText( TEXT("Spells"), FLinearColor::White, x, yPos += margin);
        DrawGroup( unit->Spells, x, yPos += margin, size, margin, 1, 1 );
      }

      yPos += size + margin;
      if( unit->Items.Num() )
      {
        DrawGroup( unit->Items, 20, Canvas->SizeY - size - 2*margin, size, margin, 1, 1 );
      }

      TArray< TEnumAsByte< Types > > buffs;
      
      // Go through the applied buffs
      for( int i = 0; i < unit->BonusTraits.size(); i++ )
        buffs.Push( unit->BonusTraits[i].traits.Type );
      if( buffs.Num() )
      {
        // vertical stack
        DrawGroup( buffs, 20, Canvas->SizeY - 2*size - 3*margin, size/2.f, margin, 0, 0 );
      }
    }

    yPos += size + margin;
    if( lastClickedObject->UnitsData.Spawns.Num() )
    {
      DrawText( TEXT("Spawns"), FLinearColor::White, x, yPos );
      DrawGroup( lastClickedObject->UnitsData.Spawns, x, yPos += margin, size, margin, 1, 1 );
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

void AMyHUD::DrawTopbar()
{
  UpdateDisplayedResources();

  float w = Canvas->SizeX;
  float h = Canvas->SizeY;
  
  DrawRect( FLinearColor::Black, 0, 0, w, margin );
  
  widgetGold.Label = FString::Printf( TEXT("Gold %d"), Game->gm->playersTeam->Gold );
  widgetLumber.Label = FString::Printf( TEXT("Lumber %d"), Game->gm->playersTeam->Lumber );
  widgetStone.Label = FString::Printf( TEXT("Stone %d"), Game->gm->playersTeam->Stone );

  // Topbar
  DrawWidget( widgetGold );
  DrawWidget( widgetLumber );
  DrawWidget( widgetStone );

  // Bottom bar
  DrawRect( FLinearColor::Black, 0, h - margin, w, margin );
  DrawText( statusMsg, FLinearColor::White, 0, h - margin );

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
  Super::DrawHUD();
  DrawSidebar();
  DrawTopbar();
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
  for( int i = 0 ; i < frameWidgets.size(); i++ )
  {
    FWidgetData &w = frameWidgets[i];

    // if one of the buildings 
    if( w.hit( mouse ) )
    {
      UE_LOG( LogTemp, Warning, TEXT("hit %s"), *GetEnumName( w.Type ) );
      // since widgets map only populated at program begin (not between game frames)
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
  statusMsg = "";
  
  for( int i = 0 ; i < frameWidgets.size(); i++ )
  {
    FWidgetData &w = frameWidgets[i];

    // Update the status message with the status info
    // of the item UNDER the mouse cursor
    if( w.hit( mouse ) )
    {
      FUnitsDataRow &data = Game->unitsData[ w.Type ];
      statusMsg = GetToolTip( data );
      return 1;
    }
  }

  return 0;
}

FString AMyHUD::GetToolTip( FUnitsDataRow& ud )
{
  FString tt;
  if( IsUnit( ud.Type ) )
  {
    tt = FString::Printf( TEXT("A %s: %s"), *ud.Name, *ud.Description );
  }
  else if( IsBuilding( ud.Type ) )
  {
    tt = FString::Printf( TEXT("%s costs %d %d %d"), *ud.Name, ud.GoldCost, ud.LumberCost, ud.StoneCost );
  }
  else if( IsResource( ud.Type ) )
  {
    tt = FString::Printf( TEXT("%s"), *ud.Name );
  }
  else if( IsItem( ud.Type ) )
  {
    tt = FString::Printf( TEXT("A %s %s"), *ud.Name, *ud.Description );
  }
  return tt;
}

FString AMyHUD::GetToolTip( AGameObject* go )
{
  FString tt;
  if( AResource *res = Cast<AResource>( go ) )
  {
    tt += FString::Printf( TEXT( "%s: %d" ), *res->UnitsData.Name, res->Amount );
  }
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


