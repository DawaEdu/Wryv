#include "Wryv.h"

#include "Building.h"
#include "CombatUnit.h"
#include "DrawDebugHelpers.h"
#include "FlyCam.h"
#include "GlobalFunctions.h"
#include "Goldmine.h"
#include "GroundPlane.h"
#include "ItemShop.h"
#include "PlayerControl.h"
#include "Projectile.h"
#include "Resource.h"
#include "Stone.h"
#include "TheHUD.h"
#include "Tree.h"
#include "Unit.h"
#include "Widget.h"
#include "Widget3D.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

#include "CastSpellAction.h"

#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
//#include "Editor/UnrealEd/Public/AssetThumbnail.h"

ATheHUD::ATheHUD(const FObjectInitializer& PCIP) : Super(PCIP)
{
  LOG( "ATheHUD::ATheHUD(ctor)");
  NextAbility = Abilities::NotSet;
  Init = 0; // Have the slots & widgets been initialized yet? can only happen
  // in first call to draw.
  SkipNextMouseUp = 0;
}

void ATheHUD::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  LOG( "ATheHUD::PostInitializeComponents()");
  rendererIcon = GetComponentByName<USceneCaptureComponent2D>( this, "rendererIcon" );
  rendererIcon->TextureTarget = PortraitTexture;
  rendererMinimap = GetComponentByName<USceneCaptureComponent2D>( this, "rendererMinimap" );
  rendererMinimap->TextureTarget = MinimapTexture;
  
  ResourceIcons[ AGoldmine::StaticClass() ] = GoldIconTexture;
  ResourceIcons[ ATree::StaticClass() ] = LumberIconTexture;
  ResourceIcons[ AStone::StaticClass() ] = StoneIconTexture;

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
  StackPanel::StackPanelTexture = VoronoiBackground;
  AbilitiesPanel::BuildButtonTexture = BuildButtonTexture;
  ImageWidget::NoTextureTexture = NoTextureTexture;
  GameCanvas::MouseCursorHand = MouseCursorHand;
  GameCanvas::MouseCursorCrossHairs = MouseCursorCrossHairs;
  Controls::PauseButtonTexture = PauseButtonTexture;
  Controls::ResumeButtonTexture = ResumeButtonTexture;
  SidePanel::RightPanelTexture = RightPanelTexture;
  Minimap::MinimapTexture = MinimapTexture;
  CostWidget::CostWidgetBackground = VoronoiBackground;
  Tooltip::TooltipBackgroundTexture = VoronoiBackground;

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

void ATheHUD::SetCursorStyle( CursorType style, FLinearColor color )
{
  switch( style )
  {
    case CursorType::CrossHairs:
      ui->gameChrome->gameCanvas->cursor->SetTexture( MouseCursorCrossHairs );
      break;

    case CursorType::Hand:
    default:
      ui->gameChrome->gameCanvas->cursor->SetTexture( MouseCursorHand );
      break;
  }
  ui->gameChrome->gameCanvas->cursor->Color = color;
}

void ATheHUD::SetHitCursor()
{
  SetCursorStyle( CursorType::CrossHairs, EmptyCrosshairColor );
}

void ATheHUD::SetPointer()
{
  SetCursorStyle( CursorType::Hand, FLinearColor(1,1,1,1) );
}

void ATheHUD::SetNextAbility( Abilities nextAbility )
{
  NextAbility = nextAbility;

  // depending on the action style, set color
  switch( nextAbility )
  {
    case Abilities::Attack:
      SetCursorStyle( CrossHairs, FLinearColor::Red );
      break;
    case Abilities::Movement:
      SetCursorStyle( CrossHairs, FLinearColor::Blue );
      break;
    case Abilities::Stop:
    case Abilities::HoldGround:
    case Abilities::NotSet:
      SetPointer();
      break;
    default:
      error( FS( "Unrecognized ability %d", (int)NextAbility ) );
      break;
  }
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
  // Drop event
  if( ui->drag )
  {
    bool absorbsMouseUp = ui->drag->AbsorbsMouseUp;
    ui->drag = 0;  // Unset the drag element
    if( absorbsMouseUp ){
      info( FS( "mouseup absorbed" ) );
      return 0; // The dragged element sucks up the UP event.
    }
  }

  // Intercept mouseups when ability or spell was used, so selection doesn't go
  if( SkipNextMouseUp ) {
    SkipNextMouseUp = 0;
    return 0;
  }

  return ui->MouseUpLeft( mouse );
}

HotSpot* ATheHUD::MouseDownLeft( FVector2D mouse )
{
  ui->drag = ui->MouseDownLeft( mouse );
  if( ui->drag )
    info( FS( "The drag elt is %s", *ui->drag->Name ) );
  return ui->drag;
}

HotSpot* ATheHUD::MouseMoved( FVector2D mouse )
{
  if( !Init )  return 0;
  //if( ui && ui->drag )
  //  info( FS( "Drag element is %s", *ui->drag->Name ) );
  
  if( SkipNextMouseUp )  return 0;

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
    sel->RemoveTagged( Game->flycam->SelectedTargetName );
    sel->RemoveTagged( Game->flycam->FollowTargetName );
    if( sel->FollowTarget )
      sel->FollowTarget->RemoveTagged( Game->flycam->FollowTargetName );
    sel->RemoveTagged( Game->flycam->AttackTargetName );
    if( sel->AttackTarget )
      sel->AttackTarget->RemoveTagged( Game->flycam->AttackTargetName );
  }

  //if( Game->flycam->ghost )
  //{
  //  Game->flycam->ClearGhost();
  //}

  // Cannot select objects of these types
  SetAGameObject forbiddenTypes = { AGroundPlane::StaticClass(), AShape::StaticClass() };
  // Pull out any objects that are of forbidden types
  for( int i = (int)objects.size() - 1; i >= 0; i-- )
    if( objects[i]->IsAny( forbiddenTypes ) )
      objects.erase( objects.begin() + i );

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
    Game->flycam->MarkAsSelected( go );
    if( go->AttackTarget )
      Game->flycam->MarkAsAttack( go->AttackTarget );
    if( go->FollowTarget )
      Game->flycam->MarkAsFollow( go->FollowTarget );
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
  // If none of the objects are in the selection, there's no change.
  if( !in( Selected, objects ) )
  {
    error( FS( "None of objects [%s] were in Selected [%s], nothing to unselect",
               *GetNames( objects ), *GetNames( Selected ) ) );
    return;
  }

  for( AGameObject* go : objects )
  {
    // Remove any selectors on there
    go->RemoveTagged( Game->flycam->SelectedTargetName );

    // If I'm the only selected unit with on follow target then remove follow target selection
    // If go is the only follower / attacker in Selected, then remove the marker.
    if( go->FollowTarget )
      if( Intersection( Selected, go->FollowTarget->Followers ).size() <= 1 )
        go->RemoveTagged( Game->flycam->FollowTargetName );

    if( go->AttackTarget )
      if( Intersection( Selected, go->AttackTarget->Attackers ).size() <= 1 )
        go->RemoveTagged( Game->flycam->AttackTargetName );


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
    RenderScreen( rendererIcon, selected->Pos, selected->Radius(), camDir );
  }
}

void ATheHUD::DrawHUD()
{
  // Canvas is only initialized here.
  Super::DrawHUD();
  InitWidgets();
  RenderPortrait();
  
  // Render the minimap, only if the floor is present
  FBox box = Game->flycam->floor->GetBox();
  FVector lookPt = box.GetCenter();
  RenderScreen( rendererMinimap, lookPt, box.GetExtent().GetMax(), FVector( 0, 0, -1 ) );

  ui->SetSize( FVector2D( Canvas->SizeX, Canvas->SizeY ) );
  ui->Update( Game->gm->T ); // Ticked here, in case reflow is needed
  ui->render();

  // Overlay the lines for the minimap's view.
  vector< FVector2D > pts = ui->gameChrome->rightPanel->minimap->pts;
  for( int i = 0; i < pts.size()-1; i++ )
  {
    Canvas->K2_DrawLine( pts[i], pts[i+1], 2.f, FLinearColor::Green );
  }
  if( pts.size() > 1 )
  {
    Canvas->K2_DrawLine( pts[ pts.size()-1 ], pts[ 0 ], 2.f, FLinearColor::Green );
  }
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
void ATheHUD::RenderScreen( USceneCaptureComponent2D* renderer, FVector lookPos, float radiusWorldUnits, FVector cameraDir )
{
  UTextureRenderTarget2D* tt = renderer->TextureTarget;
  // http://stackoverflow.com/questions/3717226/
  // radiusOnScreenPX = radiusWorldUnits*SW/(tan(fov / 2) * Z);
  // ZBack = radiusWorldUnits*SW/(tan( fovy / 2 )*radiusOnScreenPX)
  // Calculate Z distance back for a given pixel radius
  // Set particular render properties & render the screen
  // to texture in w.
  float D = GetZDistance( radiusWorldUnits, tt->GetSurfaceWidth(), tt->GetSurfaceHeight(), renderer->FOVAngle );
  FVector eyePos = lookPos - cameraDir * D;
  FQuat quat = cameraDir.Rotation().Quaternion();
  renderer->SetRelativeLocationAndRotation( eyePos, quat );
  
  FVector2D screenSize = ui->gameChrome->gameCanvas->Size;
  screenSize.X -= ui->gameChrome->rightPanel->Size.X;

  FVector up = renderer->GetUpVector();
  FLookAtMatrix lookAt( eyePos, lookPos, up );
  FPerspectiveMatrix persp( rendererMinimap->FOVAngle/2.f, 1.f, 1.f, 0.5f );
  FMatrix mvp1 = lookAt * persp;

  vector<Ray> rays = Game->pc->GetFrustumRays( FBox2DU( 0.f, 0.f, screenSize.X, screenSize.Y ) );
  float zValue = lookPos.Z;
  FPlane plane( FVector(0.f, 0.f, 1.f), zValue );
  vector<FVector> pts;
  for( int i = 0; i < rays.size(); i++ )
  {
    FVector pt = FMath::LinePlaneIntersection( rays[i].start, rays[i].end, plane );
    //Game->flycam->DrawDebug( pt, 25.f, FLinearColor::White, .25f );
    pts.push_back( pt );
  }

  //FLinearColor Cyan(0,1,1,1);
  //for( int i = 0; i < pts.size() - 1; i++ )
  //{
  //  Game->flycam->DrawDebug( pts[i], pts[i+1], 25.f, Cyan, .25f );
  //}
  //if( pts.size() > 1 )
  //{
  //  Game->flycam->DrawDebug( pts[pts.size()-1], pts[0], 25.f, Cyan, .25f );
  //}

  ui->gameChrome->rightPanel->minimap->pts.clear();
  FVector2D minimapSize = ui->gameChrome->rightPanel->minimap->Size;
  for( int i = 0; i < pts.size(); i++ )
  {
    FVector4 transformedPt = mvp1.TransformPosition( pts[i] );
    float div = transformedPt.W;
    transformedPt /= FVector4( div, div, div, div );
    FVector2D p( transformedPt.X, transformedPt.Y ); // between [-1,1]
    p *= 4.f/3.f; //!! Multiplying P by 4./3 req'd.. double-check
    p *= minimapSize/2.f;
    p.Y *= -1.f;
    p += minimapSize/2.f;

    p += ui->gameChrome->rightPanel->minimap->GetAbsPos();
    ui->gameChrome->rightPanel->minimap->pts.push_back( p );
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

