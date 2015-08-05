#pragma once

#include "Wryv.h"
#include <vector>
#include <string>
using namespace std;

enum NetMessage
{
  GameStart, // The game has started
  Play,  // A PLAY event
  Pause, // Someone paused the game
  GameEnd
};

// Each unit only has a few things it can do..
enum NetMessageCommand
{
  Move,Attack,
  Use,Special
};

// A strictly packed message, 4 byte aligned
// the extra #pragma instructions are so that the compiler doesn't use
// extra byte padding on this structure (to make it 4 or 8  byte aligned)
//#pragma push( ?? )
struct Message
{
  NetMessageCommand cmd;
};
struct GameStartMessage : public Message
{
  int Seed;
};
// This type of message is a play command
struct GamePlayMessage : public Message
{
  int unitId;  // The integer identifier of the unit receiving the command
  int command; // The command for the unit to execute
  int index;   // Use & Special each have indices (which index of item to use)
};
//#pragma pop

class Net
{
public:
  //const static FString ServerIP = "127.0.0.1";
  //const static ServerPort = 7070;
  FSocket* socket;

  Net()
  {
    // open network connection to the server
    open( "127.0.0.1", 7070 );

    // wait for a game.

    // set the random number seed & load the map indicated by the server.

    // begin play.
  }

  void recv( uint8* packet )
  {
    
  }
  
  void open( const FString& ipString, int32 port )
  {
	  FIPv4Address ipv4;
    FIPv4Address::Parse( ipString, ipv4 );
	  // The endpoint is IpAddress:port combination
    FIPv4Endpoint endPt( ipv4, port );
    // Create the socket
	  socket = FUdpSocketBuilder( "socket" ).
      BoundToEndpoint( endPt ).
      WithBroadcast().
      WithSendBufferSize( 1024 ). /// sizeable packets
      WithReceiveBufferSize( 1024 );
    
  }
};