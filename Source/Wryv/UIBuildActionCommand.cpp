#include "Wryv.h"

#include "Building.h"
#include "UIBuildActionCommand.h"
#include "Flycam.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "TheHUD.h"
#include "Peasant.h"
#include "WryvGameInstance.h"

#include "IText.h"

UUIBuildActionCommand::UUIBuildActionCommand( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

UTexture* UUIBuildActionCommand::GetIcon()
{
  return Game->GetData( BuildingType ).Portrait;
}

bool UUIBuildActionCommand::Click()
{
  // flycam places ghost of building to build next.
  Peasant->UseBuild( UUICmdActionIndex );
  return 1;
}

bool UUIBuildActionCommand::Hover()
{
  Game->hud->Status( FS( "%s [%s]", *Text, *ShortcutKey.ToString() ) );
  return 1;
}
