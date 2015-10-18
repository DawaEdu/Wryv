#include "Wryv.h"

#include "Building.h"
#include "CombatUnit.h"
#include "FlyCam.h"
#include "GameCanvas.h"
#include "Peasant.h"
#include "PlayerControl.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

#include "Action.h"
#include "BuildAction.h"
#include "CastSpellAction.h"
#include "InProgressBuilding.h"
#include "InProgressUnit.h"
#include "ItemAction.h"
#include "TrainingAction.h"
#include "UnitAction.h"

FCursorTexture GameCanvas::MouseCursorHand;
FCursorTexture GameCanvas::MouseCursorCrossHairs;

GameCanvas::GameCanvas( FVector2D size ) : Screen( "GameCanvas", size )
{
  selectBox = new MouseSelectBox( "mouse's select box",
    FBox2DU( FVector2D(100, 100), FVector2D(50, 50) ),
    4.f, FLinearColor::Green);
  Add( selectBox );
  selectBox->Hide();

  cursor = new ImageWidget( "mouse cursor", MouseCursorHand.Texture );
  Add( cursor );

  // Attach functionality
  OnMouseDownLeft = [this]( FVector2D mouse )
  {
    FHitResult hitResult = Game->pc->RayPickSingle( Game->flycam->getMousePos() );
    AGameObject* hit = Cast<AGameObject>( hitResult.GetActor() );
    if( !hit ) {
      warning( FS( "No object was clicked by the mouse" ) );
      return NotConsumed;
    }

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
    for( AGameObject* go : Game->hud->Selected )
    {
      switch( Game->hud->NextAbility )
      {
        case Abilities::Movement:
          // Explicit movement, without attack possible.
          if( hit == Game->flycam->floor )
            go->GoToGroundPosition( hitResult.ImpactPoint );
          else
            go->Follow( hit );  // otherwise, follow clicked unit
          Game->hud->SkipNextMouseUp = 1;
          break;
        case Abilities::Attack:
          // Attack only, even friendly units.
          if( hit == Game->flycam->floor )
            go->AttackGroundPosition( hitResult.ImpactPoint );  // ready to attack enemy units
          else
            go->Attack( hit );
          Game->hud->SkipNextMouseUp = 1;
          break;
        case Abilities::Stop:
          // Stops units from moving
          go->Stop();
          break;
        case Abilities::HoldGround:
          go->HoldGround();
          break;
        default:
          error( "Ability NotSet" );
          break;
      }
    }

    if( Game->hud->NextAbility )
    {
      // an ability was set, use it & revert to normal cursor
      Game->hud->SetNextAbility( Abilities::NotSet );
      return Consumed;
    }

    if( Game->flycam->ghost )
    {
      // Presence of the ghost indicates a building is queued to be placed by the UI.
      ABuilding *ghost = Game->flycam->ghost;

      // the src peasant must be present
      APeasant *peasant = 0;
      if( Game->hud->Selected.size() )
      {
        peasant = Cast<APeasant>( Game->hud->Selected[0] );
      }
      if( !peasant )
      {
        error( FS( "A peasant wasn't selected to place the building.", *peasant->GetName() ) );
        return NotConsumed;
      }

      // If the user has UI-placed the building in an acceptable spot,
      // then we should place the building here.
      if( ghost->CanBePlaced() )
      {
        // Build a building @ location. The peasant will pick up this command next frame.
        //Command cmd( Command::CommandType::CreateBuilding,
        //  peasant->ID, peasant->LastBuildingID, ghost->Pos );
        //Game->EnqueueCommand( cmd );
        peasant->UseBuild( peasant->LastBuildIndex );
        Game->flycam->ClearGhost();
      }
      else
      {
        // Ghost cannot be placed
        LOG( "Cannot place building here" );
      }
    }
    else // No ghost is set
    {
      // Box-shaped selection here
      SelectStart( mouse );
    }
    return Consumed;
  };

  OnMouseUpLeft = [this]( FVector2D mouse ) {
    // Box shaped selection of units.
    // Filtration is done after selection,
    Game->hud->Select( Game->pc->FrustumPick( selectBox->Box ) );
    SelectEnd();
    return Consumed;
  };

  OnHover = [this]( FVector2D mouse ) {
    // Regular hover event
    Set( mouse ); // Moves the mouse cursor to where px coords are.

    // If spell queued, highlight crosshairs red, else revert to default crosshairs color
    if( Game->hud->NextAbility == Abilities::Attack  ||  Game->hud->NextSpell ) 
    {
      // Highlight what's under the mouse.
      FHitResult hitResult = Game->pc->RayPickSingle( mouse );
      AGameObject* hit = Cast<AGameObject>( hitResult.GetActor() );
      if( hit != Game->flycam->floor )
        Game->hud->SetCursorStyle( ATheHUD::CursorType::CrossHairs, Game->hud->HitCrosshairColor );
      else
        Game->hud->SetCursorStyle( ATheHUD::CursorType::CrossHairs, Game->hud->EmptyCrosshairColor );
    }

    return NotConsumed;
  };

  OnMouseDragLeft = [this]( FVector2D mouse ) {
    DragBox( mouse );
    return Consumed;
  };
}

void GameCanvas::Set( FVector2D mouse )
{
  // Sets with mouse cursor position
  cursor->Margin = mouse;
}

void GameCanvas::SelectStart( FVector2D mouse )
{
  selectBox->SetStart( mouse );
  selectBox->Show();
  cursor->Hide();
}

void GameCanvas::DragBox( FVector2D mouse )
{
  selectBox->SetEnd( mouse );
}

void GameCanvas::SelectEnd()
{
  selectBox->Hide();
  cursor->Show();
}
  
