#include "Wryv.h"

#include "GameObjects/Buildings/Building.h"
#include "GameObjects/Units/CombatUnit.h"
#include "GameObjects/Things/Explosion.h"
#include "UE4/Flycam.h"
#include "UI/HotSpot/GameScreens/GameCanvas.h"
#include "GameObjects/Units/Peasant.h"
#include "UE4/PlayerControl.h"
#include "UE4/TheHUD.h"
#include "GameObjects/Units/Unit.h"
#include "GameObjects/Things/Widget3D.h"
#include "UE4/WryvGameInstance.h"

#include "UI/UICommand/Command/UIActionCommand.h"
#include "UI/UICommand/Command/UIBuildActionCommand.h"
#include "UI/UICommand/Command/UICastSpellActionCommand.h"
#include "UI/UICommand/Counters/UIInProgressBuildingCounter.h"
#include "UI/UICommand/Counters/UIInProgressUnitCounter.h"
#include "UI/UICommand/Command/UIItemActionCommand.h"
#include "UI/UICommand/Command/UITrainingActionCommand.h"
#include "UI/UICommand/Command/UIUnitActionCommand.h"

FCursorTexture GameCanvas::MouseCursorHand;
FCursorTexture GameCanvas::MouseCursorCrossHairs;

GameCanvas::GameCanvas( FVector2D size ) : Screen( "GameCanvas", size )
{
  selectBox = new MouseSelectBox( "mouse's select box",
    FBox2DU( FVector2D(100, 100), FVector2D(50, 50) ),
    4.f, FLinearColor::Green);
  Add( selectBox );
  selectBox->Hide();

  cursor = new Image( "mouse cursor", MouseCursorHand.Texture );
  Add( cursor );

  // Attach functionality to the GameCanvas
  OnMouseDownLeft = [this]( FVector2D mouse )
  {
    FHitResult hitResult = Game->pc->RayPickSingle(
      mouse, MakeSet( Game->hud->Selectables ),
      MakeSet( Game->hud->Unselectables ) );
  
    // First check if there's a queued spell or action.
    if( Game->hud->NextSpell )
    {
      // Cast the spell using any selected units that can invoke the spell
      for( AGameObject* go : Game->hud->Selected )
      {
        if( ACombatUnit* cu = Cast<ACombatUnit>( go ) )
        {
          cu->CastSpell( Game->hud->NextSpell->Spell, hitResult.ImpactPoint );
          // Revert cursor to pointer
          Game->hud->SetCursorStyle( ATheHUD::Hand, FLinearColor::White );
        }
      }
    }

    // Get the mouse location of the click
    if( Game->hud->NextAbility )
    {
      int numGridPos = FMath::CeilToInt( sqrtf( Game->hud->Selected.size() ) );
      vector<FVector> positions = Game->flycam->GenerateGroundPositions( hitResult.ImpactPoint, numGridPos );
      for( int i = 0; i < Game->hud->Selected.size(); i++ )
      {
        if( AUnit* unit = Cast< AUnit >( Game->hud->Selected[i] ) )
        {
          AGameObject* t = Cast<AGameObject>( hitResult.GetActor() );
          unit->UseAbility( Game->hud->NextAbility, t, positions[i] );
        }
      }

      // an ability was set, use it & revert to normal cursor
      Game->hud->SetNextAbility( Abilities::NotSet );
      return Consumed;
    }

    if( ! Game->flycam->IsBuildingBeingPlaced() )
    {
      BoxSelectStart( mouse );
    }
    else // Building to be placed is set
    {
      // Presence of the ghost indicates a building is queued to be placed by the UI.
      ABuilding* newBuilding = Game->flycam->ghost;

      // the src peasant must be present
      APeasant* peasant = Game->hud->GetFirstSelected<APeasant>();
      if( ! peasant )
      {
        // Peasant wasn't selected to place the building. Err-out.
        error( FS( "A peasant wasn't selected to place the building." ) );
        Game->flycam->ClearGhost();
        return NotConsumed;
      }

      // If the user has UI-placed the building in an acceptable spot,
      // then we should place the building here.
      else if( newBuilding->CanBePlaced() )
      {
        // Build a building @ location. The peasant will pick up this command next frame.
        //Command cmd( Command::CommandType::CreateBuilding,
        //  peasant->ID, peasant->LastBuildingID, ghost->Pos );
        //Game->EnqueueCommand( cmd );
        // The ghost has the building type to make
        
        Game->flycam->ClearGhost();
      }
      else
      {
        // Ghost cannot be placed
        LOG( "Cannot place building here" );
      }
    }
    return Consumed;
  };

  OnMouseUpLeft = [this]( FVector2D mouse ) {
    // Box shaped selection of units.
    // Filtration is done after selection,
    Game->hud->Select( Game->pc->FrustumPick( selectBox->Box,
      MakeSet( Game->hud->Selectables ), MakeSet( Game->hud->Unselectables ) ) );
    BoxSelectEnd();
    return Consumed;
  };

  OnHover = [this]( FVector2D mouse ) {
    // Regular hover event
    Set( mouse ); // Moves the mouse cursor to where px coords are.

    // If spell queued, highlight crosshairs red, else revert to default crosshairs color
    if( Game->hud->NextAbility == Abilities::Attack  ||  Game->hud->NextSpell ) 
    {
      // Highlight what's under the mouse.
      FHitResult hitResult = Game->pc->RayPickSingle( mouse, {}, { AShape::StaticClass() } );
      AGameObject* hit = Cast<AGameObject>( hitResult.GetActor() );
      if( hit != Game->flycam->floor )
        Game->hud->SetCursorStyle( ATheHUD::CursorType::CrossHairs, Game->hud->HitCrosshairColor );
      else
        Game->hud->SetCursorStyle( ATheHUD::CursorType::CrossHairs, Game->hud->EmptyCrosshairColor );
    }

    return NotConsumed;
  };

  OnMouseDragLeft = [this]( FVector2D mouse ) {
    BoxDrag( mouse );
    return Consumed;
  };

  OnMouseDownRight = [this]( FVector2D mouse ) {
    // Unset any buildings on right-click.
    if( Game->flycam->ghost )
    {
      info( FS( "The building %s was cancelled", *Game->flycam->ghost->Stats.Name ) );
      Game->flycam->ClearGhost();
    }
    else if( !Game->hud->Selected.size() )
    {
      info( "Nothing to command" );
    }
    else
    {
      Game->flycam->Target();
    }
    return NotConsumed;
  };
}

void GameCanvas::Set( FVector2D mouse )
{
  // Sets with mouse cursor position
  cursor->Margin = mouse;
}

void GameCanvas::BoxSelectStart( FVector2D mouse )
{
  selectBox->SetStart( mouse );
  selectBox->Show();
  cursor->Hide();
}

void GameCanvas::BoxDrag( FVector2D mouse )
{
  selectBox->SetEnd( mouse );
}

void GameCanvas::BoxSelectEnd()
{
  selectBox->Hide();
  cursor->Show();
}
  
