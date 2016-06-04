#include "Wryv.h"

#include "Command.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "TheHUD.h"
#include "Shape.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

UWryvGameInstance* Game = 0;

UWryvGameInstance::UWryvGameInstance(const FObjectInitializer& PCIP) : Super(PCIP)
{
  LOG( "UWryvGameInstance::UWryvGameInstance()");
  defaultSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  NextObjectID = 1;
  // On construction of first game object, initialize game instance
  Game = Cast<UWryvGameInstance>( this );
  hud = 0;
  pc = 0;
  gm = 0;
  gs = 0;
  flycam = 0;
  IsDestroyStarted = 0;
}

void UWryvGameInstance::SetCommand( const Command& cmd )
{
  //info( FS( "SetCommand %s", *cmd.ToString() ) );
  ClearFlags(); // Clear waypointed flags
  /// retrieve command list from object
  AGameObject* go = GetUnitById( cmd.srcObjectId );
  if( !go )
  {
    error( FS( "Unit %d doesn't exist, command `%s` is invalid", cmd.srcObjectId, *cmd.ToString() ) );
    return ;
  }
  
  // Remove old queued from go from global history
  commands -= go->commands;

  // Pop current command and replace with the new one
  go->commands.clear();
  go->commands.push_back( cmd );
  go->IsReadyToRunNextCommand = 1;
}
  

void UWryvGameInstance::EnqueueCommand( const Command& cmd )
{
  ClearFlags(); // Clear waypointed flags
  info( FS( "Enqueued Command %s", *cmd.ToString() ) );
  commands.push_back( cmd );
  AGameObject* unit = GetUnitById( cmd.srcObjectId );
  if( !unit )
  {
    error( FS( "Unit %d not found", cmd.srcObjectId ) );
    return;
  }

  unit->commands.push_back( cmd );

  // Add in a waypoint flag when commanded unit is 1st in hud selection 
  if( Game->hud->Selected.size()   &&   unit == Game->hud->Selected[0] )
  {
    unit->DisplayWaypoints();
  }
}

// Clear waypointed flags
void UWryvGameInstance::ClearFlags()
{
  for( pair< const int64, AShape* > p : Flags )
  {
    p.second->Cleanup();
  }
  Flags.clear();
}

void UWryvGameInstance::ClearFlag( int64 cmdId )
{
  map< int64, AShape* >::iterator it = Flags.find( cmdId );
  if( it != Flags.end() )
  {
    it->second->Cleanup();
    Flags.erase( it );
  }
}

int64 UWryvGameInstance::NextId()
{
  return NextObjectID++;
}

AGameObject* UWryvGameInstance::GetUnitById( int64 unitId )
{
  for( int i = 0; i < gm->teams.size(); i++ )
    for( int j = 0; j < gm->teams[i]->units.size(); j++ )
      if( gm->teams[i]->units[j]->ID == unitId )
        return gm->teams[i]->units[j];

  error( FS( "Unit %d doesn't exist", unitId ) );
  // Dump instance names / ids for debug
  for( int i = 0; i < gm->teams.size(); i++ )
    for( int j = 0; j < gm->teams[i]->units.size(); j++ ) {
      AGameObject* unit = gm->teams[i]->units[j];
      LOG( "  Unit ID %d is %s", unit->ID, *unit->GetName() );
    }
  return 0;
}

// A check function. You should avoid using this function whenever possible
bool UWryvGameInstance::IsReady()
{
  if( !IsDestroyStarted && // Not shutting down
       gm && gs && pc && hud && flycam // stock objects
    )
    return 1;
  else  error( "GAME NOT READY" );
  return 0;
}

void UWryvGameInstance::Init()
{
  LOG( "UWryvGameInstance::Init()");
  // Calls more than once per startup.
  Super::Init();
}

// This only happens ONE TIME in the startup (fortunately)
ULocalPlayer*	UWryvGameInstance::CreateInitialPlayer(FString& OutError)
{
  LOG( "UWryvGameInstance::CreateInitialPlayer()");
  return Super::CreateInitialPlayer( OutError );
}

void UWryvGameInstance::StartGameInstance()
{
  Super::StartGameInstance();
  LOG( "UWryvGameInstance::StartGameInstance()");
}

bool UWryvGameInstance::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Out)
{
  LOG( "UWryvGameInstance::Exec()");
  return Super::Exec( InWorld, Cmd, Out );
}

FUnitsData UWryvGameInstance::GetData( UClass* ClassType )
{
  if( !ClassType )
  {
    error( "Cannot GetData() for NULL ClassType" );
  }
  //else if( !in( BaseUnitsData, ClassType ) )
  else if( BaseUnitsData.find( ClassType ) == BaseUnitsData.end() )
  {
    // Add it in
    info( FS( "First load of UClass `%s`", *ClassType->GetName() ) );
    AGameObject* object = Make<AGameObject>( ClassType );
    if( object ) {
      BaseUnitsData[ ClassType ] = object->BaseStats;
      object->Die();
      object->Cleanup();
    }
    else  error( FS( "Couldn't make object ClassType `%s`", *ClassType->GetName() ) );
  }
  return BaseUnitsData[ ClassType ];
}

UTexture* UWryvGameInstance::GetPortrait( UClass* ClassType ) {
  return GetData( ClassType ).Portrait;
}

void UWryvGameInstance::BeginDestroy()
{
  IsDestroyStarted = 1;
  Super::BeginDestroy();
}


