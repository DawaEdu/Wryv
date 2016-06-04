#include "Wryv.h"
#include "GlobalFunctions.h"
#include "Unit.h"
#include "WryvGameInstance.h"
#include "WryvGameMode.h"

AWryvGameMode::AWryvGameMode(const FObjectInitializer& PCIP) : Super( PCIP )
{
  LOG("AWryvGameMode::AWryvGameMode()");
  PrimaryActorTick.bCanEverTick = 1;
  T = 1.f / 60.f;
  GameSpeed = 1.f;
  tick = 0;
}

void AWryvGameMode::InitGame( const FString& MapName, const FString& Options, FString& ErrorMessage )
{
  Super::InitGame( MapName, Options, ErrorMessage );
  LOG( "AWryvGameMode::InitGame(%s, %s, %s)", *MapName, *Options, *ErrorMessage );
  
  if( TeamColors.Num() < 8 )
  {
    // push in random colors
    warning( "TeamColors not set up..  using random colors instead" );
    for( int i = 0; i < 8 ; i++ )
    {
      TeamColors.Push( FLinearColor::MakeRandomColor() );
    }
  }

  // Create Teams required for this map. This happens before AActor::BeginPlay().
  Game->gm = this;
  neutralTeam = new Team( 0, "Neutral", Alliances::Neutral, TeamColors[0] );
  teams.push_back( neutralTeam ); 
  playersTeam = new Team( 1, "Player", Alliances::Friendly, TeamColors[1] );
  teams.push_back( playersTeam );
  enemyTeam = new Team( 2, "Opponent", Alliances::Enemy, TeamColors[2] );
  teams.push_back( enemyTeam );
}

void AWryvGameMode::StartPlay()
{
  Super::StartPlay();
  LOG( "AWryvGameMode::StartPlay()");
  // Call level-load related functions

  // Initialize ALL gameobjects that were thrown into teams.
  for( int i = 0; i < teams.size(); i++ )
  {
    // Update the team's AI by evaluating the map.
    teams[i]->OnMapLoaded();
  }
}

void AWryvGameMode::StartMatch()
{
  Super::StartMatch();
  LOG( "AWryvGameMode::StartMatch()");
}

void AWryvGameMode::SetMatchState(FName NewState)
{
  Super::SetMatchState( NewState );
}

void AWryvGameMode::Tick(float DeltaSeconds)
{
  Super::Tick( DeltaSeconds );

  if( !Game->IsReady() )
  {
    LOG( "AWryvGameMode::Tick(): Game not ready" );
    return;
  }

  // ::Tick() function progresses the game. Progress all Team AI.
  tick++;
  Time += T; // Increase total gametime elapsed.

  // Go through teams in deterministic order.
  // (player 1 is first, player 2 is second.. etc).
  for( int i = 0; i < teams.size(); i++ )
  {
    // Update the team's AI by evaluating the map.
    teams[i]->Move( T ); // USE FIXED TIMESTEP
  }
}

vector<AGameObject*> AWryvGameMode::GetObjectsOfType( UClass* ClassType )
{
  vector<AGameObject*> v;

  // Get all objects in the level of this type
  ULevel* level = GetLevel(); // The level containing the GameMode object contains the
  // Actors in the level.
  TTransArray<AActor*>& actors = level->Actors;
  for( int i = 0; i < actors.Num(); i++ )
  {
    AGameObject* go = Cast<AGameObject>( actors[i] );
    if( go->IsA( ClassType ) )
      v.push_back( go );
  }

  return v;
}

Alliance AWryvGameMode::GetAlliance( Alliances alliance )
{
  Alliance all;
  all.alliance = alliance;
  // throw in all teams with same alliance
  for( Team* team : teams )
    if( team->alliance == alliance )
      all.teams.push_back( team );
  return all;
}

AActor* AWryvGameMode::Find( FString name )
{
  ULevel* level = GetWorld()->GetLevel(0);
  TTransArray<AActor*> *actors = &level->Actors;
  for( int i = 0; i < actors->Num(); i++ )
  {
    AActor* a = (*actors)[i];
    if( a == nullptr )  continue;
    if( a->GetName() == name )  return a;
  }
  return 0; // not found
}

void AWryvGameMode::BeginDestroy()
{
  for( Team* team : teams )
    delete team;
  teams.clear();
  Super::BeginDestroy();  // PUT THIS LAST or the object may become invalid
}