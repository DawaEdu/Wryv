#include "Wryv.h"
#include "Command.h"
#include "FlyCam.h"
#include "GameObject.h"
#include "TheHUD.h"
#include "Shape.h"
#include "UnitTypeUClassPair.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

UWryvGameInstance* Game = 0;

UWryvGameInstance::UWryvGameInstance(const FObjectInitializer& PCIP) : Super(PCIP)
{
  LOG( "UWryvGameInstance::UWryvGameInstance()");
  NextObjectID = 1;
  // On construction of first game object, initialize game instance
  Game = Cast<UWryvGameInstance>( this );
  hud = 0;
  pc = 0;
  gm = 0;
  gs = 0;
  flycam = 0;
  UClassesLoaded = 0;
  IsDestroyStarted = 0;
}

void UWryvGameInstance::SetCommand( const Command& cmd )
{
  ClearFlags(); // Clear waypointed flags
  /// retrieve command list from object
  AGameObject* go = GetUnitById( cmd.srcObjectId );
  if( !go )
  {
    error( FS( "Unit %d doesn't exist", cmd.srcObjectId ) );
    return ;
  }
  
  /// Remove old queued from go from global history
  commands -= go->commands;
  
  go->commands.clear();
  go->commands.push_back( cmd );
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
  if( Game->hud->Selected.size() && unit == Game->hud->Selected[0] )
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
       gm && gs && pc && hud && flycam && // stock objects
       UClassesLoaded ) // Game->unitsData has been loaded.
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

void UWryvGameInstance::AssertIntegrity()
{
  // Assert integrity of base stats of each unit type.
  for( pair<Types,FUnitsDataRow> p : unitsData )
  {
    FUnitsDataRow ud = p.second;
    ud.Type = p.first;

    if( ud.ReleasedProjectileWeapon && !IsProjectile( ud.ReleasedProjectileWeapon ) )
    {
      error( FS( "%s had a projectile of type %s", *ud.Name, *GetTypesName( ud.ReleasedProjectileWeapon ) ) );
      ud.ReleasedProjectileWeapon = Types::PROJWEAPONARROW;
    }

    // Object spawned when exploded cannot be the same as this object
    if( ud.OnExploded != Types::NOTHING   &&   ud.Type == ud.OnExploded )
    {
      ud.Type = p.first;
      error( FS( "OBJECT TYPE `%s` spawns same as self (%s) OnExploded",
        *GetTypesName( ud.Type ), *GetTypesName( ud.OnExploded ) ) );
    }
  }
}

void UWryvGameInstance::LoadUClasses()
{
  // Connect Widgets & Stats with mappings from Types
  map<Types,UClass*> classes;
  for( int i = 0; i < UnitTypeUClasses.Num(); i++ )
  {
    int type = UnitTypeUClasses[i].Type;
    UClass* uClass = UnitTypeUClasses[i].uClass;
    if( !uClass )
    {
      error( FS( "LoadUClasses: entry %d => uClass=NULL in UnitTypeUClasses", i ) );
      skip;
    }
    else if( type >= Types::MAX ) {
      error( FS( "BAD ENTRY %d => class %s", type, *uClass->GetName() ) );
      skip;
    }

    info( FS( "entry %d => class (%s)", type, *uClass->GetName() ) );
    classes[ (Types)type ] = uClass;
  }

  // Fix the array so we're sure it is in order
  UnitTypeUClasses.SetNum( Types::MAX );
  // Init all with the NOTHING object
  for( int i = 0; i < UnitTypeUClasses.Num(); i++ )
  {
    UnitTypeUClasses[i].Type = NOTHING;
    UnitTypeUClasses[i].uClass = classes[ NOTHING ];
  }

  for( pair<Types,UClass*> p : classes )
  {
    UnitTypeUClasses[ (int)p.first ].Type = p.first;
    UnitTypeUClasses[ (int)p.first ].uClass = p.second;
  }

  vector< Types > types;
  for( int i = 0; i < Types::MAX; i++ )
    types.push_back( (Types)i );
  
  // Pull all the UCLASS names from the Blueprints we created.
  for( pair<Types,UClass*> p : classes )
  {
    // create a unit of that type from the blueprint to extract the properties
    // that were entered inside blueprints
    
    Types type = p.first;
    UClass* uClass = p.second;

    if( type < 0 || type >= Types::MAX ) {
      error( FS( "Type %d is OOB defined types", (int)type ) );
      type = NOTHING;
    }

    if( type == Types::FOGOFWAR ) skip; // The fogofwar doesn't inherit from gameobject

    // Cannot call Make here since Teams are not ready. We don't want a version of Make
    // that doesn't assign a team since team assignment is vital and easy to forget.
    AGameObject* unit = GetWorld()->SpawnActor<AGameObject>( uClass, FVector(0.f), FRotator(0.f) );
    if( !unit )
    {
      error( FS( "No blueprint class for type `%s` was specified", *GetTypesName( type ) ) );
      skip;
    }

    LOG( "Type %s => Blueprint (%s)", *GetTypesName( type ), *p.second->GetName() );
    unit->BaseStats.uClass = GetUClass( type );   // Write the uClass associated with the type here
    Game->unitsData[ type ] = unit->BaseStats;    // 
    unit->Cleanup();                              // destroy the sample unit
  }

  AssertIntegrity();
}

// This only happens ONE TIME in the startup (fortunately)
ULocalPlayer*	UWryvGameInstance::CreateInitialPlayer(FString& OutError)
{
  LOG( "UWryvGameInstance::CreateInitialPlayer()");
  // Pull up the UnitTypeUClasses. The UnitTypeUClasses mapping just connects each
  // Types:: object to its UE4 UClass.
  LoadUClasses();
  UClassesLoaded = 1; // This only calls once, so that's why I hit the bool init here
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

void UWryvGameInstance::BeginDestroy()
{
  IsDestroyStarted = 1;
  Super::BeginDestroy();
}


