#include "Wryv.h"
#include "Command.h"

int64 Command::NextCommandID = 1;

Command::Command() : 
  CommandID( NextCommandID++ ), FrameID( 0 ),
  commandType( CommandType::GoToGroundPosition ), srcObjectId( 0 ), 
  targetObjectId( 0 ), buildingType( BLDGBARRACKS ), pos( 0,0,0 ) { }

Command::Command( CommandType cmd, int64 iSrcObjectId ) :
  CommandID( NextCommandID++ ), FrameID( 0 ),
  commandType( cmd ), srcObjectId( iSrcObjectId ), 
  targetObjectId( 0 ), buildingType( BLDGBARRACKS ), pos( 0,0,0 ) { }

Command::Command( CommandType cmd, int64 iSrcObjectId, FVector iLocation ) :
  CommandID( NextCommandID++ ), FrameID( 0 ),
  commandType( cmd ), srcObjectId( iSrcObjectId ), 
  targetObjectId( 0 ), buildingType( BLDGBARRACKS ), pos( iLocation ) { }

Command::Command( CommandType cmd, int64 iSrcObjectId, int64 iTargetObjectId ) :
  CommandID( NextCommandID++ ), FrameID( 0 ),
  commandType( cmd ), srcObjectId( iSrcObjectId ), 
  targetObjectId( iTargetObjectId ), buildingType( BLDGBARRACKS ), pos(0,0,0) { }

FString Command::ToString() const
{
  map<CommandType, FString> cmds = {
    { CommandType::Target, "Target" }, { CommandType::GoToGroundPosition, "GoToGroundPosition" },
    { CommandType::Build, "Build" } };
  FString str = FS( "[CMDID=%d] Type=`%s` srcObjectId=%d", CommandID, *cmds[ commandType ], srcObjectId );
  if( commandType == CommandType::Build )
    str += FS( "%s", *GetTypesName( buildingType ) );
  str += FS( " (%f, %f, %f)", pos.X, pos.Y, pos.Z );
  return str;
}

