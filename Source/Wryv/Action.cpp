#include "Wryv.h"

#include "Action.h"
#include "Clock.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "ITextWidget.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

UAction::UAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

UTexture* UAction::GetIcon()
{
  error( FS( "GetIcon not implemented in %s", *GetClass()->GetName() ) );
  return 0;
}

void UAction::Step( float t )
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
    clock->SetFillFraction( cooldown.Fraction() );

    // Completion only once
    if( cooldown.Done() )
    {
      //info( "Cooldown done" );
      OnCooldown(); //Cooldown is completed.
    }
  }
}

void UAction::PopulateClock( Clock* inClock, int index )
{
  clock = inClock;

  // Recall cooldown's time from the action object (model object)
  // Set the cooldown's time at Populate time. Too early to do in ctor 
  // (UClass data may not have loaded)
  cooldown.TotalTime = GetCooldownTotalTime();
  
  // attach Action's Click() function to clicking on the clock.
  UActionIndex = index; // index inside the array that hosts the Counters.
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

void UAction::OnCooldown()
{
  //info( FS( "Action %s has completed", *Text ) );
}

