#include "Wryv.h"

#include "Building.h"
#include "CombatUnit.h"
#include "DrawDebugHelpers.h"
#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "GroundPlane.h"
#include "ItemShop.h"
#include "PlayerControl.h"
#include "Projectile.h"
#include "Resource.h"
#include "TheHUD.h"
#include "Unit.h"
#include "UnitsData.h"
#include "Widget.h"
#include "Widget3D.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
//#include "Editor/UnrealEd/Public/AssetThumbnail.h"

FName ATheHUD::AttackTargetName = "AttackTarget";
FName ATheHUD::FollowTargetName = "FollowTarget";
FName ATheHUD::SelectedTargetName = "SelTarget";

ATheHUD::ATheHUD(const FObjectInitializer& PCIP) : Super(PCIP)
{
  LOG( "ATheHUD::ATheHUD(ctor)");
  Init = 0; // Have the slots & widgets been initialized yet? can only happen
  // in first call to draw.
}

void ATheHUD::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  LOG( "ATheHUD::PostInitializeComponents()");
  rendererIcon = GetComponentByName<USceneCaptureComponent2D>( this, "rendererIcon" );
  rendererMinimap = GetComponentByName<USceneCaptureComponent2D>( this, "rendererMinimap" );
}

void ATheHUD::BeginPlay()
{
  Super::BeginPlay();
  LOG( "ATheHUD::BeginPlay()");
}

void ATheHUD::InitWidgets()
{
  if( Init ) return;
  Init = 1;
  LOG( "InitWidgets()" );

  // Initialize the widgets that show the player gold, lumber, stone counts.
  ResourcesWidget::GoldTexture = GoldIconTexture;
  ResourcesWidget::LumberTexture = LumberIconTexture;
  ResourcesWidget::StoneTexture = StoneIconTexture;
  SolidWidget::SolidWhiteTexture = SolidWhiteTexture;
  SlotPalette::SlotPaletteTexture = SlotPaletteTexture;
  StackPanel::StackPanelTexture = StackPanelTexture;
  AbilitiesPanel::BuildButtonTexture = BuildButtonTexture;
  ImageWidget::NoTextureTexture = NoTextureTexture;
  GameCanvas::MouseCursorHand = MouseCursorHand;
  GameCanvas::MouseCursorCrossHairs = MouseCursorCrossHairs;
  Controls::PauseButtonTexture = PauseButtonTexture;
  Controls::ResumeButtonTexture = ResumeButtonTexture;
  SidePanel::RightPanelTexture = RightPanelTexture;
  Minimap::MinimapTexture = MinimapTexture;
  CostWidget::CostWidgetBackground = TooltipBackgroundTexture;
  Tooltip::TooltipBackgroundTexture = TooltipBackgroundTexture;

  FVector2D canvasSize( Canvas->SizeX, Canvas->SizeY );
  ui = new UserInterface( canvasSize );

  // Create the panel for containing items/inventory
  // Map selection screen
  MapSelectionScreen *mss = ui->mapSelectionScreen = 
    new MapSelectionScreen( TitleLogoTexture, SolidWhiteTexture,
      MapSlotEntryBackgroundTexture, PortraitTexture,
      canvasSize, FVector2D( 120, 24 ), largeFont );
  ui->titleScreen = new TitleScreen( TitleScreenTexture, canvasSize );
  ui->Add( mss );
  mss->OKButton->OnMouseDownLeft = [mss](FVector2D mouse) -> EventCode {
    // OK button clicked, so load the map if there is a selected widget
    // else display error message
    if( mss->Selected )
      Game->flycam->LoadLevel( FName( *mss->Selected->GetText() ) );
    else
      Game->hud->ui->statusBar->Set( "Select a map to load first" ) ;
    return NotConsumed;
  };

  /////
  // List the maps in the folder at the left side
  TArray<FAssetData> maps = ScanFolder( "/Game/Maps" );
  for( int i = 0; i < maps.Num(); i++ )
    ui->mapSelectionScreen->AddText( maps[i].AssetName.ToString(), CenterCenter );
  ui->missionObjectivesScreen = new MissionObjectivesScreen(
    MapSlotEntryBackgroundTexture, MapSlotEntryBackgroundTexture, canvasSize,
    FVector2D( 300, 100 ), FVector2D( 8, 8 ) );
  ui->Add( ui->missionObjectivesScreen );

  /// Set the screen's to correct one for the gamestate
  ui->SetScreen( Game->gm->state );
}

TArray<FAssetData> ATheHUD::ScanFolder( FName folder )
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
    LOG( "%s", *assets[i].AssetName.ToString() );
  }
  return assets;
}

HotSpot* ATheHUD::MouseUpLeft( FVector2D mouse )
{
  ui->drag = 0;
  return ui->MouseUpLeft( mouse );
}

HotSpot* ATheHUD::MouseDownLeft( FVector2D mouse )
{
  ui->drag = ui->MouseDownLeft( mouse );
  return ui->drag;
}

HotSpot* ATheHUD::MouseMoved( FVector2D mouse )
{
  if( !Init )  return 0;

  // Drag events are when the left mouse button is down.
  if( Game->pc->IsKeyDown( EKeys::LeftMouseButton ) )
  {
    // pass the event to the drag element if any, 
    // if there is no drag element, then you cannot have a drag event here
    if( ui->drag )
      ui->drag->MouseDraggedLeft( mouse );
  }
  else
    return ui->Hover( mouse );  // Check against all ui widgets that require hover.

  return 0;
}

void ATheHUD::Select( vector<AGameObject*> objects )
{
  // Remove all selection markers (previous selection)
  for( AGameObject* sel : Selected )
  {
    RemoveTagged( sel, SelectedTargetName );
    RemoveTagged( sel, FollowTargetName );
    if( sel->FollowTarget )  RemoveTagged( sel->FollowTarget, FollowTargetName );
    RemoveTagged( sel, AttackTargetName );
    if( sel->AttackTarget )  RemoveTagged( sel->AttackTarget, AttackTargetName );
  }

  //if( Game->flycam->ghost )
  //{
  //  Game->flycam->ClearGhost();
  //}

  // Cannot select objects of these types
  set< TSubclassOf<AGameObject> > forbiddenTypes = { AGroundPlane::StaticClass(), AWidget3D::StaticClass() };
  function< bool (AGameObject*) > filter = [ forbiddenTypes ]( AGameObject *go ) -> bool {
    // remove objects of forbidden types.
    return go->Dead || go->IsAny( forbiddenTypes );
  };
  objects |= filter;

  if( Game->pc->IsAnyKeyDown( { EKeys::LeftShift, EKeys::RightShift } ) )
    Selected += objects; //Add
  else if( Game->pc->IsAnyKeyDown( { EKeys::LeftControl, EKeys::RightControl } ) )
    Selected -= objects; //Subtract
  else
    Selected = objects;  //Replace old selection

  // create & parent the selectors to all selected objects
  for( AGameObject * go : Selected )
  {
    // make an attack target if there is an attack target for the gameobject
    MarkAsSelected( go );
    if( go->AttackTarget )  MarkAsAttack( go->AttackTarget );
    if( go->FollowTarget )  MarkAsFollow( go->FollowTarget );
  }

  // Modify the UI to reflect selected gameobjects
  ui->gameChrome->Select( Selected );

  // Run its OnSelected function, playing sounds etc.
  for( AGameObject* go : Selected )
    go->OnSelected();

  // Clear old waypointing flags
  Game->ClearFlags();
  // Take the front object and display waypointing if any
  if( Selected.size() )
  {
    Selected[0]->DisplayWaypoints();
  }
}

void ATheHUD::Unselect( vector<AGameObject*> objects )
{
  // Filter THIS from collection if exists
  for( AGameObject* go : objects )
  {
    // Remove any selectors on there
    RemoveTagged( go, SelectedTargetName );

    // If I'm the only selected unit with on follow target then remove follow target selection
    // If go is the only follower / attacker in Selected, then remove the marker.
    if( go->FollowTarget )
      if( Intersection( Selected, go->FollowTarget->Followers ).size() <= 1 )
        RemoveTagged( go, FollowTargetName );

    if( go->AttackTarget )
      if( Intersection( Selected, go->AttackTarget->Attackers ).size() <= 1 )
        RemoveTagged( go, AttackTargetName );
  }

  Selected -= objects;

  //Reselect selected, to refresh the selectors on attack targets.
  Select( Selected );
}

void ATheHUD::Status( FString msg )
{
  ui->statusBar->Set( msg );
  info( msg );
}

void ATheHUD::MarkAsSelected( AGameObject* object )
{
  if( HasChildWithTag( object, SelectedTargetName ) ) {
    LOG( "%s already has a %s", *object->Stats.Name, *SelectedTargetName.ToString() );
    return;
  }

  AWidget3D* widget = Game->Make<AWidget3D>( SelectorClass, object->team );
  if( !widget ) { error( "Widget3d couldn't be created" ); return; }
  
  widget->Tags.Add( SelectedTargetName );
  float r = object->Radius() * 1.5f;
  widget->SetActorScale3D( FVector(r,r,r) );
  widget->SetMaterialColors( "Color", FLinearColor(0,1,0,1) );
  object->AddChild( widget );
}

void ATheHUD::MarkAsFollow( AGameObject* object )
{
  // only select as follow target if its not already an attack target of something
  // ( attack priorities over follow )
  if( HasChildWithTag( object, FollowTargetName ) ) {
    //LOG( "%s already marked as follow", *object->Stats.Name );
    return; // already marked as an attack target
  }

  AWidget3D* widget = Game->Make<AWidget3D>( SelectorClass, object->team );
  if( !widget ) { error( "Widget3d couldn't be created" ); return; }
  
  widget->Tags.Add( FollowTargetName );
  float r = object->Radius() * 1.5f;
  widget->SetActorScale3D( FVector(r,r,r) );
  widget->SetMaterialColors( "Color", FLinearColor(1,1,0,1) );
  object->AddChild( widget );
}

void ATheHUD::MarkAsAttack( AGameObject* object )
{
  // Check that it doesn't already have a Selector-typed child
  if( HasChildWithTag( object, AttackTargetName ) ) {
    //LOG( "%s already marked as attack", *object->Stats.Name );
    return; // already marked as an attack target
  }

  AWidget3D* widget = Game->Make<AWidget3D>( SelectorClass, object->team );
  if( !widget ) { error( "Widget3d couldn't be created" ); return; }
  
  widget->Tags.Add( AttackTargetName );
  float r = object->Radius() * 1.5f;
  widget->SetActorScale3D( FVector( r,r,r ) );
  widget->SetMaterialColors( "Color", FLinearColor(1,0,0,1) );
  object->AddChild( widget );
}

void ATheHUD::DrawMaterial(UCanvas* Canvas, UMaterialInterface* Material, float ScreenX, float ScreenY, 
  float ScreenW, float ScreenH, float MaterialU, float MaterialV, float MaterialUWidth, float MaterialVHeight)
{
  FCanvasTileItem TileItem( FVector2D( ScreenX, ScreenY ), Material->GetRenderProxy(0), 
    FVector2D( ScreenW, ScreenH ) * 1.f, FVector2D( MaterialU, MaterialV ), 
    FVector2D( MaterialU + MaterialUWidth, MaterialV + MaterialVHeight ) );
	TileItem.Rotation = FRotator(0, 0, 0);
	TileItem.PivotPoint = FVector2D::ZeroVector;
	Canvas->DrawItem( TileItem );
}

void ATheHUD::DrawMaterial(UCanvas* Canvas, UMaterialInterface* Material, float ScreenX, float ScreenY, 
  float ScreenW, float ScreenH, float MaterialU, float MaterialV, float MaterialUWidth, float MaterialVHeight, 
  float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot)
{
  FCanvasTileItem TileItem( FVector2D( ScreenX, ScreenY ), Material->GetRenderProxy(0), 
    FVector2D( ScreenW, ScreenH ) * Scale, FVector2D( MaterialU, MaterialV ), 
    FVector2D( MaterialU + MaterialUWidth, MaterialV + MaterialVHeight ) );
	TileItem.Rotation = FRotator(0, Rotation, 0);
	TileItem.PivotPoint = RotPivot;
	Canvas->DrawItem( TileItem );
}

void ATheHUD::DrawTexture(UCanvas* canvas, UTexture* Texture, float ScreenX, float ScreenY,
  float ScreenW, float ScreenH, float TextureU, float TextureV,
  float TextureUWidth, float TextureVHeight, FLinearColor Color,
  EBlendMode BlendMode, float Scale, bool bScalePosition, float Rotation, FVector2D RotPivot)
{
	if (IsCanvasValid_WarnIfNot() && Texture)
	{
		FCanvasTileItem TileItem( FVector2D( ScreenX, ScreenY ), Texture->Resource, FVector2D( ScreenW, ScreenH ) * Scale, FVector2D( TextureU, TextureV ), FVector2D( TextureU + TextureUWidth, TextureV + TextureVHeight ), Color );
		TileItem.Rotation = FRotator(0, Rotation, 0);
		TileItem.PivotPoint = RotPivot;
		if (bScalePosition)
		{
			TileItem.Position *= Scale;
		}
		TileItem.BlendMode = FCanvas::BlendToSimpleElementBlend( BlendMode );
		Canvas->DrawItem( TileItem );
	}
}

void ATheHUD::RenderPortrait()
{
  // Draws the portrait of the first selected object
  if( Selected.size() )
  {
    AGameObject* selected = *Selected.begin();
    // Portrait: render last-clicked object to texture zoom back by radius of bounding sphere of clicked object
    FVector camDir( .5f, .5f, -FMath::Sqrt( 2.f ) );
    RenderScreen( rendererIcon, PortraitTexture, 
      selected->Pos, selected->Radius(), camDir );
  }
}

void ATheHUD::DrawHUD()
{
  // Canvas is only initialized here.
  Super::DrawHUD();
  HotSpot::hud = this;
  InitWidgets();
  RenderPortrait();
  
  // Render the minimap, only if the floor is present
  FBox box = Game->flycam->floor->GetBox();
  FVector p = box.GetCenter();
  RenderScreen( rendererMinimap, MinimapTexture, p, box.GetExtent().GetMax(), FVector( 0, 0, -1 ) );
  
  ui->SetSize( FVector2D( Canvas->SizeX, Canvas->SizeY ) );
  ui->Move( Game->gm->T ); // Ticked here, in case reflow is needed
  ui->render();
}

void ATheHUD::Tick( float t )
{
  Super::Tick( t );
}

// Returns to you the distance that you should set
// the camera away from tt to get a render on tt
// that has RadiusOnScreen
float ATheHUD::GetZDistance( float radiusWorldUnits, float radiusOnScreenPx, float texW, float fovyDegrees )
{
  // ZBack = radiusWorldUnits*screenWidthPX /
  //         (tan( fovy / 2 )*radiusOnScreenPX)
  float den = tanf( FMath::DegreesToRadians( fovyDegrees / 2.f ) ) * radiusOnScreenPx;
  return radiusWorldUnits*texW / den;
}

float ATheHUD::GetPxWidth( float radiusWorldUnits, float distanceToObject, float texW, float fovyDegrees )
{
  // radiusOnScreenPX = radiusWorldUnits*screenWidthPX / 
  //                    (tan( fovy / 2 )*distanceToObject)
  float den = tanf( FMath::DegreesToRadians( fovyDegrees / 2.f ) )*distanceToObject;
  return radiusWorldUnits*texW / den;
}

// Render onto tt (using renderer) sitting @ cameraPos,
// facing cameraDir, an object with radiusWorldUnits.
void ATheHUD::RenderScreen( USceneCaptureComponent2D* renderer,
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

  for( int i = 0; i < renderer->AttachChildren.Num(); i++ )
  {
    USceneComponent* c = renderer->AttachChildren[i];
    LOG( "Component [%d] = %s", i, *renderer->AttachChildren[i]->GetName() );
  }
}

void ATheHUD::BeginDestroy()
{
  //LOG( "ATheHUD::BeginDestroy()");
  if( ui ) {
    delete ui;
    ui = 0;
  }
  Super::BeginDestroy();  // PUT THIS LAST or the object may become invalid
}

