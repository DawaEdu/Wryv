#include "Wryv.h"

#include "Clock.h"
#include "TheHUD.h"
#include "UserInterface.h"
#include "WryvGameInstance.h"

UserInterface::UserInterface( FVector2D size ) :
  Screen( "UI-root", size )
{
  // Initialize the status bar before gamechrome, since statusbar height is used
  statusBar = new StatusBar( FLinearColor::Black );
  Add( statusBar );
  HotSpot::TooltipWidget = statusBar->Text;
  info( FS( "Status bar height %f", statusBar->Size.Y ) );
  
  gameChrome = new GameChrome( "GameChrome", size );
  gameChrome->buildQueue->Margin.Y = statusBar->Size.Y;
  gameChrome->itemBelt->Margin.Y = statusBar->Size.Y;
  Add( gameChrome );
  gameChrome->Select( {} ); // empty selection

  // Adjust the BuildQueue's margin 
  titleScreen = 0;
  mapSelectionScreen = 0;
  missionObjectivesScreen = 0;
  drag = 0;
  dirty = 0;
  // Clock materials stopped drawing in 4.9
  //clock = new Clock( "clock", FVector2D(200,200), Game->hud->GoldIconTexture, FLinearColor::Black );
  //clock->Margin = FVector2D( 200,200 );
  //clock->Set( 0.25, Alignment::CenterCenter );
  //gameChrome->Add( clock );


}

void UserInterface::SetScreen( int mode )
{
  // hide elts not to be shown
  titleScreen->Hide();
  gameChrome->Hide();
  mapSelectionScreen->Hide();
  missionObjectivesScreen->Hide();
    
  // Then go ahead and show elts depending on mode we're in
  if( mode == Title )  titleScreen->Show();
  else if( mode == MapSelect )  mapSelectionScreen->Show();
  else if( mode == Running )  gameChrome->Show();

  statusBar->Show(); // Show the status bar at all times
}

void UserInterface::SetSize( FVector2D size )
{
  Size = size;
  gameChrome->Size = size;
}

void UserInterface::Update( float t )
{
  if( dirty )
  {
    // Refresh.
    gameChrome->Select( gameChrome->Selected );
    dirty = 0;
  }

  // Clock test, for testing clock material in ue 4.9
  //static float tr = 0.f;
  //tr+=t;
  //clock->Set( tr / 10.f, Alignment::CenterCenter );
}


