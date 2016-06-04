#include "Wryv.h"

#include "UIActionCommand.h"
#include "Clock.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "IText.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

UUIActionCommand::UUIActionCommand( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

UTexture* UUIActionCommand::GetIcon()
{
  error( FS( "GetIcon not implemented in %s", *GetClass()->GetName() ) );
  return 0;
}

void UUIActionCommand::Step( float t )
{
  if( !cooldown.TotalTime )
  {
    // Not a timed action.
    return;
  }

  // Only step the counter if the cooldown isn't complete yet.
  // Actions with no cooldown don't enter this Step function
  if( !cooldown.Done() )
  {
    cooldown.Step( t );
    
    // Cooldown update.
    if( clock ) {
      clock->SetFillFraction( cooldown.Fraction() );
    }
    else {
      info( FS( "Clock for %s not set", *GetName() ) );
    }

    // Completion only once
    if( cooldown.Done() )
    {
      //info( "Cooldown done" );
      OnCooldown(); //Cooldown is completed.
    }
  }
}

void UUIActionCommand::PopulateClock( Clock* inClock, int index )
{
  clock = inClock;

  // Recall cooldown's time from the action object (model object)
  // Set the cooldown's time at Populate time. Too early to do in ctor 
  // (UClass data may not have loaded)
  cooldown.TotalTime = GetCooldownTotalTime();
  
  // attach Action's Click() function to clicking on the clock.
  UUICmdActionIndex = index; // index inside the array that hosts the Counters.
  clock->SetTexture( GetIcon() );
  
  // Set the cooldown amount to being the cooldown of the particular action
  clock->SetFillFraction( cooldown.Fraction() );
  
  //info( FS( "Populating clock `%s` with cooldown=%f/%f",
  //  *clock->Name, cooldown.Time, cooldown.TotalTime ) );
  clock->Show();

  // Invoke I'th action of the object
  clock->OnMouseDownLeft = [this]( FVector2D mouse ) -> EventCode { 
    Click();
    info( FS( "Clicked Action %s", *GetName() ) );
    return Consumed;
  };
  
  clock->OnHover = [this]( FVector2D mouse ) -> EventCode {
    Hover();
    return NotConsumed;
  };
}

void UUIActionCommand::OnCooldown()
{
  //info( FS( "Action %s has completed", *Text ) );
}

