#include "Wryv.h"
#include "Command.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

int64 Command::NextCommandID = 1;

Command::Command() : 
  CommandID( 0 ), // NULL command
  FrameID( 0 ),   // Game->gm typically unavailable at construction
  commandType( CommandType::GoToGroundPosition ), srcObjectId( 0 ), 
  targetObjectId( 0 ), pos( 0,0,0 ) { }

Command::Command( CommandType cmd, int64 iSrcObjectId ) :
  CommandID( NextCommandID++ ), FrameID( Game->gm->tick ),
  commandType( cmd ), srcObjectId( iSrcObjectId ), 
  targetObjectId( 0 ), pos( 0,0,0 ) { }

Command::Command( CommandType cmd, int64 iSrcObjectId, FVector iLocation ) :
  CommandID( NextCommandID++ ), FrameID( Game->gm->tick ),
  commandType( cmd ), srcObjectId( iSrcObjectId ), 
  targetObjectId( 0 ), pos( iLocation ) { }

Command::Command( CommandType cmd, int64 iSrcObjectId, int64 iTargetObjectId ) :
  CommandID( NextCommandID++ ), FrameID( Game->gm->tick ),
  commandType( cmd ), srcObjectId( iSrcObjectId ), 
  targetObjectId( iTargetObjectId ), pos(0,0,0) { }

Command::Command( CommandType cmd, int64 iSrcObjectId, int64 iTargetObjectId, FVector iLocation ) :
  CommandID( NextCommandID++ ), FrameID( Game->gm->tick ),
  commandType( cmd ), srcObjectId( iSrcObjectId ), 
  targetObjectId( iTargetObjectId ), pos(0,0,0)
{
}

FString Command::ToString() const
{
  map<CommandType, FString> cmds = { { CommandType::Target, "Target" },
    { CommandType::GoToGroundPosition, "GoToGroundPosition" },
    { CommandType::CreateBuilding, "CreateBuilding" } };
  FString str = FS( "[%d] [CMDID=%d] Type=`%s` srcObjectId=%d", 
    FrameID, CommandID, *cmds[ commandType ], srcObjectId );
  if( commandType == CommandType::CreateBuilding )
    str += FS( "%s", *GetTypesName( (Types)targetObjectId ) );
  str += FS( " (%f, %f, %f)", pos.X, pos.Y, pos.Z );
  return str;
}

