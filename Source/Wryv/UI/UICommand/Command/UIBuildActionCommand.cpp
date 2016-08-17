#include "Wryv.h"

#include "UI/UICommand/Command/UIBuildActionCommand.h"
#include "GameObjects/Buildings/Building.h"
#include "GameObjects/GameObject.h"
#include "GameObjects/Units/Peasant.h"
#include "Util/GlobalFunctions.h"
#include "UE4/Flycam.h"
#include "UE4/TheHUD.h"
#include "UE4/WryvGameInstance.h"
#include "UI/HotSpot/Elements/IText.h"

UUIBuildActionCommand::UUIBuildActionCommand( const FObjectInitializer & PCIP ) : Super( PCIP )
{
}

UTexture* UUIBuildActionCommand::GetIcon()
{
  return Game->GetData( BuildingClass ).Portrait;
}

bool UUIBuildActionCommand::Click()
{
  return Peasant->UseBuild( UUICmdActionIndex );
}

bool UUIBuildActionCommand::Hover()
{
  Game->hud->Status( FS( "%s [%s]", *Text, *ShortcutKey.ToString() ) );
  return 1;
}
