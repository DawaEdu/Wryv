#include "Wryv.h"

#include "Building.h"
#include "BuildAction.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "GlobalFunctions.h"
#include "TheHUD.h"
#include "Peasant.h"
#include "WryvGameInstance.h"

#include "ITextWidget.h"

UBuildAction::UBuildAction( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

UTexture* UBuildAction::GetIcon()
{
  return Game->GetData( BuildingType ).Portrait;
}

bool UBuildAction::Click()
{
  // flycam places ghost of building to build next.
  Peasant->UseBuild( UActionIndex );
  return 1;
}

bool UBuildAction::Hover()
{
  Game->hud->Status( FS( "%s [%s]", *Text, *ShortcutKey.ToString() ) );
  return 1;
}
