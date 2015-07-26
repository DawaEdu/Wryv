#include "RTSGame.h"
#include "RTSGameInstance.h"
#include "RTSGameGameMode.h"
#include "GameObject.h"
#include "MyHUD.h"
#include "FlyCam.h"

URTSGameInstance* Game = 0;

URTSGameInstance::URTSGameInstance(const FObjectInitializer& PCIP) : Super(PCIP)
{
  UE_LOG( LogTemp, Warning, TEXT("URTSGameInstance::URTSGameInstance()") );
  
  // On construction of first game object, initialize game instance
  Game = Cast<URTSGameInstance>( this );
  init = 0;
  myhud = 0;
  pc = 0;
  gm = 0;
  gs = 0;
  flycam = 0;
  tick = 0; // Global tick #
  IsDestroyStarted = 0;
}

bool URTSGameInstance::IsReady()
{
  return !IsDestroyStarted && init && myhud && pc && gm && gs && 
    flycam && flycam->pathfinder;
}

void URTSGameInstance::BeginDestroy()
{
  Super::BeginDestroy();
  IsDestroyStarted = 1;
}

AGameObject* URTSGameInstance::Make( Types type, FVector v, int teamId )
{
  UClass* uc = unitsData[type].uClass;
  FRotator ro( 0.f );
  AGameObject* go = (AGameObject*)GetWorld()->SpawnActor<AGameObject>( uc, v, ro );
  if( go )
  {
    go->UnitsData.Team = teamId;
  }
  else
  {
    UE_LOG( LogTemp, Warning, TEXT("URTSGameInstance::Make() couldn't make object %s"), *uc->GetName() );
  }
  return go;
}

ARTSGameGameMode* URTSGameInstance::GetGameMode()
{
  UWorld *w = GetWorld();
  if( w )  return (ARTSGameGameMode*)w->GetAuthGameMode();
  else  return 0;
}

void URTSGameInstance::Init()
{
  Super::Init();

  if( init )  return;
  UE_LOG( LogTemp, Warning, TEXT("URTSGameInstance::Init()") );

  if( !DataTable )
  {
    UE_LOG( LogTemp, Warning, TEXT("Datatable not initialized") );
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
        //UE_LOG( LogTemp, Warning, TEXT("row reads: %s"), *fud->ToString() );
      }
      
      unitsData[ fud->Type.GetValue() ] = *fud;
    }
  }
  init = 1;
}

ULocalPlayer*	URTSGameInstance::CreateInitialPlayer(FString& OutError)
{
  UE_LOG( LogTemp, Warning, TEXT("URTSGameInstance::CreateInitialPlayer()") );
  Init();
  return Super::CreateInitialPlayer( OutError );
}

void URTSGameInstance::StartGameInstance()
{
  Super::StartGameInstance();
  UE_LOG( LogTemp, Warning, TEXT("URTSGameInstance::StartGameInstance()") );
}

bool URTSGameInstance::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Out)
{
  UE_LOG( LogTemp, Warning, TEXT("URTSGameInstance::Exec()") );
  return Super::Exec( InWorld, Cmd, Out );
}


