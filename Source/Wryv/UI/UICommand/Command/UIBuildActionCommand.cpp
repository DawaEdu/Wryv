#include "Wryv.h"

#include "GameObjects/Buildings/Building.h"
#include "UI/UICommand/Command/UIBuildActionCommand.h"
#include "UE4/Flycam.h"
#include "GameObjects/GameObject.h"
#include "Util/GlobalFunctions.h"
#include "UE4/TheHUD.h"
#include "GameObjects/Units/Peasant.h"
#include "UE4/WryvGameInstance.h"

#include "UI/HotSpot/Elements/IText.h"

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
