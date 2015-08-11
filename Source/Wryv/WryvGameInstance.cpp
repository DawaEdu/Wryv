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
  return !IsDestroyStarted && init && hud && pc && gm && gs && flycam;
}

void UWryvGameInstance::BeginDestroy()
{
  Super::BeginDestroy();
  IsDestroyStarted = 1;
}

AGameObject* UWryvGameInstance::Make( Types type, FVector v, int teamId )
{
  UClass* uc = unitsData[type].uClass;
  FRotator ro( 0.f );
  AGameObject* go = (AGameObject*)GetWorld()->SpawnActor<AGameObject>( uc, v, ro );
  if( go )
  {
    go->Stats.Team = teamId;
  }
  else
  {
    LOG( "UWryvGameInstance::Make() couldn't make object %s", *uc->GetName() );
  }
  return go;
}

AWryvGameMode* UWryvGameInstance::GetGameMode()
{
  UWorld *w = GetWorld();
  if( w )  return (AWryvGameMode*)w->GetAuthGameMode();
  else  return 0;
}

void UWryvGameInstance::Init()
{
  init = 1;  //flag for IsReady() to know if instance ready or not

  Super::Init();

  // Pull up the UnitTypeUClasses. The UnitTypeUClasses mapping just connects each
  // Types:: object to its UE4 UClass.
  LoadUClasses();

  LOG( "UWryvGameInstance::Init()");

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
  for( int i = 0; i < Types::MAX; i++ )  types.push_back( (Types)i );
  
  // Pull all the UCLASS names from the Blueprints we created.
  for( int i = 0; i < UnitTypeUClasses.Num(); i++ )
  {
    Types ut = UnitTypeUClasses[i].Type;
    UClass* uc = UnitTypeUClasses[i].uClass;
    if( !uc )  continue;
    LOG( "Loaded unit=%s / UClass=%s",
      *GetEnumName( ut ), *uc->GetClass()->GetName() );
    
    // create a unit of that type from the blueprint to extract the properties
    // that were entered inside blueprints
    AGameObject* unit = (AGameObject*)GetWorld()->SpawnActor( uc );
    if( ! unit )
    {
      LOG( "  Couldn't load: %s", *uc->GetClass()->GetName() );
      continue;
    }

    // Save in all the defaults that blueprint specifies,
    // including spawn costs, etc. We need this data here
    // because we need to check if we have enough money
    // to spawn a unit of a certain type before spawning it.
    unit->Stats.uClass = uc;  // Not available in dropdowns from table selector.
    Game->unitsData[ ut ] = unit->Stats;
    
    //LOG( "Loaded unit: %s", *unit->Stats.ToString() );
    unit->Destroy(); // destroy the unit 
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


