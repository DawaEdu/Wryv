#pragma once

class AGameObject;

// This structure contains a single command for a unit.
// Units can stack these in a series
struct Command
{
  enum CommandType { Target, GoTo, Build };
  CommandType commandType;
  int64 objectId;
  FVector loc;

  Command() : commandType( CommandType::GoTo ), objectId( 0 ), loc( 0,0,0 )
  {
  }

  Command( const Command& cmd ) :
    commandType( cmd.commandType ), objectId( cmd.objectId ), loc( cmd.loc )
  {
  }

  Command( CommandType cmd, int64 iObjectId ) :
    commandType( cmd ), objectId( iObjectId ), loc( 0,0,0 )
  {
  }

  Command( CommandType cmd, int64 iObjectId, FVector iLoc ) :
    commandType( cmd ), objectId( iObjectId ), loc( iLoc )
  {
  }
};