#pragma once

class AGameObject;

// This structure contains a single command for a unit.
// Units can stack these in a series
struct Command
{
  static int64 NextCommandID;

  enum CommandType { CreateBuilding, 
    CreateUnit, 
    GoToGroundPosition, 
    Stop, 
    HoldGround,
    Target, UseAbility };

  int64 CommandID;          // The numeric ID of the Command (these are in-order)
  int64 FrameID;
  CommandType commandType;  // The type of the command added here
  int64 srcObjectId;        // The ID of the object that this command concerns
  int64 targetObjectId;     // The target object that this command concerns (for Follow, Attack, Build commands)
  FVector pos;              // Location of the command (esp if ground position)

  Command();
  Command( CommandType cmd, int64 iSrcObjectId );
  Command( CommandType cmd, int64 iSrcObjectId, FVector iLocation );
  Command( CommandType cmd, int64 iSrcObjectId, int64 iTargetObjectId );
  Command( CommandType cmd, int64 iSrcObjectId, int64 iTargetObjectId, FVector iLocation );
  // Equality by ID only.
  bool operator==( const Command& cmd ) const;
  FString ToString() const;
};