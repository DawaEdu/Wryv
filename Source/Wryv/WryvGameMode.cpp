#include "Wryv.h"
#include "WryvGameMode.h"
#include "Unit.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

AWryvGameMode::AWryvGameMode(const FObjectInitializer& PCIP) : Super( PCIP )
{
  LOG("AWryvGameMode::AWryvGameMode()");
  PrimaryActorTick.bCanEverTick = true;
  T = 1.f / 60.f;
  GameSpeed = 1.f;
  tick = 0;
}

Team* AWryvGameMode::GetTeam( int32 teamId )
{
  Team* team = teams[ teamId ];
  check( team && "Team not existing" );
  return team;
}

void AWryvGameMode::InitGame( const FString& MapName, const FString& Options, FString& ErrorMessage )
{
  Super::InitGame( MapName, Options, ErrorMessage );
  LOG( "AWryvGameMode::InitGame(%s, %s, %s)", *MapName, *Options, *ErrorMessage );
  
  // Create Teams required for this map.
  // This happens before AActor::BeginPlay().
  Game->gm = this;
  neutralTeam = teams[ 0 ] = new Team( 0, "Neutral" );
  teams[ 0 ]->alliance = Alliance::Neutral;
  playersTeam = teams[ 1 ] = new Team( 1, "Player" );
  teams[ 1 ]->alliance = Alliance::Friendly;
  enemyTeam = teams[ 2 ] = new Team( 2, "Opponent" );
  teams[ 2 ]->alliance = Alliance::Enemy;
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

vector<AGameObject*> AWryvGameMode::GetObjectsOfType( Types type )
{
  vector<AGameObject*> v;

  // Get all objects in the level of this type
  ULevel* level = GetLevel();
  TTransArray<AActor*> *actors = &level->Actors;
  for( int i = 0; i < actors->Num(); i++ )
  {
    AGameObject* g = Cast<AGameObject>( (*actors)[i] );
    if( g->Stats.Type == type )
      v.push_back( g );
  }

  return v;
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
  for( pair< int, Team* > p : teams )  delete p.second;

  Super::BeginDestroy();  // PUT THIS LAST or the object may become invalid
}