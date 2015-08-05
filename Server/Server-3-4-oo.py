#!C:/Python34/python.exe
from socket import *
from select import select
from struct import pack
from enum import Enum
import time
import threading
import sys
import random

#enums start @ 1
class NetMessage( Enum ):
  GameStart=1
  GameCommand=2
  GameEnd=3

""" You can construct custom match setups """
class GameStyle( Enum ):
  OneVsOne=1
  TwoVsTwo=2
  ThreeVsThree=3
  FourVsFour=4

""" Player goes from Chatting to WaitingForMatch, to Playing,
 then back to Chatting again when done the game. """
class Status( Enum ):
  Chatting=1
  WaitingForMatch=2
  Playing=3

""" Removes an item from a list if it exits, prints a warning if not """
def rm( list, item ):
  if item in list:
    list.remove( item )
  else:
    print( "rm: Item not in list" )

# the listing of stock maps (directory listing of /maps)
StockMaps = [ 'map1', 'map2', 'map3', 'map4', 'map5', 'map6', 'map7', 'map8', 'map9' ]
  
# Global logging functions.
""" Redirects stdout to point to .txt file so print statements go there """
def OpenLog( filename ):
  sys.stdout = open( 'py-server-log.txt', 'w' )

""" Logs information with timestamp """
def Log( val ):
  print( time.strftime( '%c' ),': ',val,'\n' )
  sys.stdout.flush()
#</def Log>

""" Sends a data packet down to all connected peers, and self """
def Multicast( sender, peers, data ):
  for peer in peers:
    # if peer != sender # don't send to self.
    peer.send( data )
    

""" Turns a tuple into a string. %a just throws any type into a string. """
def String( tuple ):
  return '%a, ' * len( tuple[:-1] ) % tuple[:-1] + tuple[-1]

def ExcString( exc ):
  return '%s %s %s' % exc

""" Runs a game instance """
class GameInstance:
  players = [] # an array of sockets.
  ids = {} # a dictionary mapping player sockets to player ids.
  seed = 1

  """ Initialize a new game, given an array of sockets. """
  def __init__( self, sockets ):
    self.players = sockets

    # assign player ids
    i = 1
    for s in sockets:
      ids[ s ] = i
      i += 1

    # startup a new thread to run the game itself.
    threading.Thread( target = self.run, args=() ).start()

  # struct.pack makes a binary packet, https://docs.python.org/3.0/library/struct.html
  def createStartMessage( self, playerId ):
    return pack( 'iii', NetMessage.GameStart, playerId, self.seed )
  
  def createGameEndMessage( self, winnerId ):
    return pack( 'ii', NetMessage.GameEnd, winnerId )

  """ Starts the game """
  def gameStart( self ):
    # create player objects for each socket passed
    self.seed = random.randint( 1, 44100 )
    Log( 'Created randomseed = %d' % self.seed )
    for p in players:
      p.send( self.createStartMessage( self.ids[ p ] ) )
    Log( 'Sent start messages to clients' )
  
  def gameOver( self ):
    # send a game ended message
    for p in players:
      p.close() # or send a game-end message and let the client close the connection

  """ Defines and sends the gamestart packet
      Launch on a separate thread from main """
  def run( self ):
    self.gameStart()
    # now players know their ID's (player #) and the random number seed
    # for the game map.
    # Now we listen for data at sockets and multicast any incoming data
    # from the clients to the server.
    running = True
    while running: # LOCKS the thread as running
      ( ready, wlist, exclist ) = select( players, [], [] )
      for sock in ready:
        data = None
        try:
          data = sock.recv( 1024 ) # receive 1k of data max
        except:
          Log( 'Someone has gone away, abandoning session' )
          running = False
          break
        if data is None or len( data ) == 0:
          Log( 'A player has disconnected, abandoning session' )
          running = False
          break

        # Multicasts the data sent by a client back down to all connected players
        Multicast( sock, players, data )
      #</for sock in ready>
    #</while running>
    
    # the game instance is over here
    self.gameOver()
  #/run (Thread exit)
#


""" A class defining player game preferences. Used only in the Chatroom,
 where the player is selecting and waiting for his next match. """
class PlayerPrefs:
  gameStyle = GameStyle.OneVsOne # The selected game style to play (1v1, 2v2, 3v3 etc)
  status = Status.Chatting
  
class Chatroom:
  playerPrefs = {} # players connected in the chatroom. { socketId1 : Player(), socketId2 : Player() }
  waitingPools = {} # player game wait pools.
  # A dictionary like { GameStyle.OneVsOne:[players waiting for 1v1], GameStyle.TwoVsTwo:[players waiting for 2v2] }
  Running = True
  
  def __init__( self ):
    # Initialize the waitingPools to be empty
    self.waitingPools[ GameStyle.OneVsOne ] = []
    self.waitingPools[ GameStyle.TwoVsTwo ] = []
    self.waitingPools[ GameStyle.ThreeVsThree ] = []
    self.waitingPools[ GameStyle.FourVsFour ] = []
    # begin running the chatroom
    threading.Thread( target = self.run ).start()
  
  # Add a player to the chatroom. Construct a Player object for him
  # and allow him to select match parameters
  def add( self, socket ):
    self.playerPrefs[ socket ] = PlayerPrefs()
    # he's not in a waiting pool yet
    
  # a player socket has sent a request to change his gameStyle
  def setGameStyle( self, socket, gameStyle ):
    waitingPools[ gameStyle ].append( socket )
    
  # remove a player from chatroom to put into a game.
  def remove( self, player ):
    del self.playerPrefs[ player ]
    # search the game wait pools to remove him from all of those too
    for (gameStyle, pool) in self.waitingPools.items():
      rm( pool, player )
  
  def startMatch( self, gameStyle, players ):
    # put the game on a separate thread
    pass
    
  def pullMatchUps( self, gameStyle, players ):
    playersReqd = 2 * gameStyle.value
    
    # keep pulling groups of players into games
    # until # players reqd for match too large
    # for # players available
    while playersReqd <= len( players ):
      ps = []
      for i in range( playersReqd ):
        ps.append( players.pop(0) )
      self.startMatch( gameStyle, ps )
  
  # attempt to matchup given players in waitingPools
  def findMatchups( self ):
    for (gameStyle, players) in self.waitingPools.items():
      self.pullMatchUps( gameStyle, players )
  
  def changePlayerPrefs( self, playerSocket, data ) :
    # changes player preferences
    player = self.playerPrefs[ playerSocket ]
    player.gameStyle = data
    
  """ Running the chatroom means listening to all connected clients and changing preferences appropriately,
   and slinging players into games when a match is found """
  def run( self ):
    while self.Running:
      # listen on all sockets for incoming requests to change params
      playerSockets = list( self.playerPrefs.keys() ) # grab the sockets
      if len( playerSockets ) == 0:
        continue # skip when there's no sockets to listen to
      
      ( ready, dontcare, dontcare ) = select( playerSockets, [], [] )
      for playerSocket in ready:
        # player has changed his preferences
        data = None
        try:
          data = playerSocket.recv( 1024 ) # receive 1k of data max
        except:
          Log( 'Socket %s has gone away' % playerSocket )
          self.remove( playerSocket )
          break
        if data is None or len( data ) == 0:
          Log( 'A player %s has disconnected, abandoning session' % playerSocket )
          self.remove( playerSocket )
          break
        # otherwise, here the player @ socket has changed his gameplay preferences.
        # interpret his gameplay prefs packet change and change data in playerPrefs
        self.changePlayerPrefs( playerSocket, data )
        
      # try and matchup after options change for this player
      self.findMatchups()
    #/while self.Running
    

""" Accepts incoming connects and pairs them to games """
class GameServer:
  ip = ''  # binds to all local ip addresses '127.0.0.1'
  port = 7070
  mainSock = None  # the main listener socket, used for incoming connections.
  Running = True   # set to false to shut the server down. Set to false from another thread.
  chatroom = Chatroom() # the chatroom where players set match preferences and wait for matches
  gameInstances = [] # running game instances
  
  """Starts and runs the game server"""
  def run( self ):
    Log( 'DO NOT RUN THIS PROGRAM BY PRESSING F5 FROM THE PYTHON.ORG IDE.' )
    Log( 'Instead, run it directly (double click it from Windows explorer or call python.exe server.py)' )
    
    # Re-direct stdout to file for later review
    #OpenLog( 'py-server-log.txt' )
    Log( 'Startup' )

    # Try to startup the server by creating a socket and listening.
    self.mainSock = self.openListener()
    
    # Start up sentry thread which is cmd-line means to quit
    # This unblocks the socket listener and allows the program to exit
    threading.Thread( target = self.QuitSentry ).start()
    
    # wait for incoming connections and throw all players into the chatroom
    self.Running = True
    while self.Running:
      try:
        # BLOCK the script until a player connects
        ( s, ipPort ) = self.mainSock.accept() # BLOCK
        self.chatroom.add( s )
        Log( 'A player is here, ip=%s, port=%d' % ipPort )
      except:
        Log( "I couldn't accept a player, reason: %s" % ExcString(sys.exc_info()) )
    #/while self.Running
    # close the chatroom
    self.chatroom.Running = False
    Log( 'MT exit' )

  def openListener( self ):
    s = None
    try:
      s = socket( AF_INET, SOCK_STREAM )  # 1. create server socket
      s.bind( ( self.ip, self.port ) )    # 2. bind
      s.listen( 5 )                       # 3. listen
    except:
      # Only get here if something bad happened in try block above
      exc = sys.exc_info()
      Log( "Server couldn't startup! Reason: %s" % ExcString(exc) )
      if s != None:
        s.close()
      sys.exit()  # bail if failed
    return s
  
  """ Allows you to stop the server by typing 'quit' at the command line """
  def QuitSentry( self ):
    # blocks until you type something
    cmd = input( '\nType anything and press <ENTER> to stop the server and disconnect all people\n' )
    # signal to all threads basically that we're shutting down
    Log( 'Shutting down the server...' )
    for game in self.gameInstances:
      game.gameOver()
    # Stop listening for new connections
    self.stopListening()
  #</def QuitSentry>

  """ Stop the main socket from listening for incoming connections,
      unblock & close the main socket """
  def stopListening( self ):
    self.Running = False  
    self.Unblock()
    self.mainSock.close()

  """ Unblocks the self.mainSock (causes it to return from its .accept() blocking call).
  Used only when shutting down. """
  def Unblock( self ):
    Log( 'Unblocking the main socket' )
    # Unblock main thread socket
    s = socket( AF_INET, SOCK_STREAM )
    # connect to self.mainSock to basically unblock it
    s.connect( ('127.0.0.1', 7070) )
    s.close()
  #</def Unblock>

gs = GameServer()
gs.run()
