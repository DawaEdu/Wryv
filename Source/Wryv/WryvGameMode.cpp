#include "Wryv.h"
#include "WryvGameMode.h"
#include "Unit.h"
#include "WryvGameInstance.h"
#include "GlobalFunctions.h"

AWryvGameMode::AWryvGameMode(const FObjectInitializer& PCIP) : Super( PCIP )
{
  PrimaryActorTick.bCanEverTick = true;
  LOG("AWryvGameMode::AWryvGameMode()");
  T = 1.f / 60.f;
  GameSpeed = 1.f;
  tick = 0;
}

void AWryvGameMode::BeginDestroy()
{
  //LOG( "AWryvGameMode::BeginDestroy()");
  for( pair< int, Team* > p : teams )  delete p.second;

  Super::BeginDestroy();  // PUT THIS LAST or the object may become invalid
}

void AWryvGameMode::StartPlay()
{
  LOG( "AWryvGameMode::StartPlay()");

  // In AGameObject::BeginPlay(), we have use of this object from Game->gm,
  // so it has to be initialized here.
  Game->gm = this;

  // this is before actors have ::BeginPlay() called on them.

  // Calls all ::BeginPlay()
  Super::StartPlay();
}

void AWryvGameMode::InitGame( const FString& MapName, const FString& Options, FString& ErrorMessage )
{
  Super::InitGame( MapName, Options, ErrorMessage );
  LOG( "AWryvGameMode::InitGame(%s, %s, %s)", *MapName, *Options, *ErrorMessage );

  neutralTeam = teams[ 0 ] = new Team( 0, "Neutral" ); // neutral team
  teams[ 0 ]->alliance = Alliance::Neutral;

  // We default initialization to Team 1 being the player's team
  playersTeam = teams[ 1 ] = new Team( 1, "Player" );
  teams[ 1 ]->alliance = Alliance::Friendly;

  enemyTeam = teams[ 2 ] = new Team( 2, "Opponent" );
  teams[ 2 ]->alliance = Alliance::Enemy;

  //UGameplayStatics::OpenLevel( GetWorld(), FName( "map5" ) );
}

void AWryvGameMode::Tick(float t)
{
  // ::Tick() function progresses the game. Progress all Team AI.
  tick++;
  Time += T; // Increase total gametime elapsed.
  Super::Tick( t );
  
  if( !Game->IsReady() )
  {
    return;
  }

  // Go through teams in deterministic order.
  // (player 1 is first, player 2 is second.. etc).
  for( int i = 0; i < teams.size(); i++ )
  {
    // Update the team's AI by evaluating the map.
    teams[i]->Move( t );
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
    if( g->UnitsData.Type == type )
      v.push_back( g );
  }

  return v;
}

