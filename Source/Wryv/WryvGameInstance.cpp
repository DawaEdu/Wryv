#include "Wryv.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"
#include "GameObject.h"
#include "TheHUD.h"
#include "FlyCam.h"
#include "UnitTypeUClassPair.h"

UWryvGameInstance* Game = 0;

UWryvGameInstance::UWryvGameInstance(const FObjectInitializer& PCIP) : Super(PCIP)
{
  LOG( "UWryvGameInstance::UWryvGameInstance()");
  
  // On construction of first game object, initialize game instance
  Game = Cast<UWryvGameInstance>( this );
  init = 0;
  hud = 0;
  pc = 0;
  gm = 0;
  gs = 0;
  flycam = 0;
  IsDestroyStarted = 0;
}

// A check function. You should avoid using this function whenever possible
bool UWryvGameInstance::IsReady()
{
  if( !IsDestroyStarted && init && hud && pc && gm && gs && flycam && flycam->pathfinder )
    return 1;
  else
    error( "GAME NOT READY" );
  return 0;
}

void UWryvGameInstance::Init()
{
  LOG( "UWryvGameInstance::Init()");
  Super::Init();
  init = 1;  //flag for IsReady() to know if instance ready or not

  // Pull up the UnitTypeUClasses. The UnitTypeUClasses mapping just connects each
  // Types:: object to its UE4 UClass.
  LoadUClasses();

  // Data table has bugs in it
  // ------------------------------------------------------------------------------
  return;

  if( !DataTable )
  {
    LOG( "Datatable not initialized");
    return;
  }
  else
  {
    // DataTable
    TArray<FName> rowNames = DataTable->GetRowNames();
    for( int i = 0 ; i < rowNames.Num(); i++ )
    {
      FUnitsDataRow *fud = (FUnitsDataRow *)DataTable->RowMap[ rowNames[i] ];
      
      if( fud )
      {
        //LOG( "row reads: %s", *fud->ToString() );
      }
      
      unitsData[ fud->Type.GetValue() ] = *fud;
    }
  }
}

void UWryvGameInstance::AssertIntegrity()
{
  // Assert integrity of base stats of each unit type.
  for( pair<Types,FUnitsDataRow> p : unitsData )
  {
    FUnitsDataRow ud = p.second;
    // Check contact object not same
    if( ud.OnContact != Types::NOTHING   &&   ud.Type == ud.OnContact )
    {
      error( FS( "OBJECT TYPE %s spawns same as self (%s) on Contact",
        *GetTypesName( ud.Type ), *GetTypesName( ud.OnContact ) ) );
    }
  }
}

void UWryvGameInstance::LoadUClasses()
{
  // Connect Widgets & Stats with mappings from Types
  map<Types,UClass*> classes;
  for( int i = 0; i < UnitTypeUClasses.Num(); i++ )
  {
    if( !UnitTypeUClasses[i].uClass )
    {
      LOG( "entry %d wasn't set in UnitTypeUClasses", i );
      skip;
    }
    LOG( "entry %d => class (%s)", (int)UnitTypeUClasses[i].Type, *UnitTypeUClasses[i].uClass->GetName() );
    classes[ UnitTypeUClasses[i].Type ] = UnitTypeUClasses[i].uClass;
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
    unit->Destroy();                              // destroy the sample unit
  }

  AssertIntegrity();
}

ULocalPlayer*	UWryvGameInstance::CreateInitialPlayer(FString& OutError)
{
  LOG( "UWryvGameInstance::CreateInitialPlayer()");
  Init();
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
  Super::BeginDestroy();
  IsDestroyStarted = 1;
}


