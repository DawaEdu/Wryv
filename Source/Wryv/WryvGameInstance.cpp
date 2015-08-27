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

bool UWryvGameInstance::IsReady()
{
  return !IsDestroyStarted && init && hud && pc && gm && gs && flycam && flycam->pathfinder;
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

void UWryvGameInstance::LoadUClasses()
{
  // Connect Widgets & Stats with mappings from Types
  vector< Types > types;
  for( int i = 0; i < Types::MAX; i++ )
    types.push_back( (Types)i );
  
  // Pull all the UCLASS names from the Blueprints we created.
  for( int i = 0; i < UnitTypeUClasses.Num(); i++ )
  {
    Types type = UnitTypeUClasses[i].Type;
    // create a unit of that type from the blueprint to extract the properties
    // that were entered inside blueprints
    AGameObject* unit = Make<AGameObject>( type );
    if( !unit )
    {
      fatal( FS( "Couldn't load: %s", *GetTypesName( type ) ) );
      continue;
    }

    unit->BaseStats.uClass = GetUClass( type );   // Write the uclass associated with the type here
    Game->unitsData[ type ] = unit->BaseStats;    // 
    unit->Destroy();                              // destroy the sample unit
  }
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


