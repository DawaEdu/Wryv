#include "Wryv.h"

#include "GlobalFunctions.h"
#include "PlayerStartPosition.h"
#include "WryvGameMode.h"

APlayerStartPosition::APlayerStartPosition( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, "flag1" );
  Mesh->AttachTo( GetRootComponent() );
  TeamNumber = 0;
}

void APlayerStartPosition::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  aiProfile = Construct<UAIProfile>( this, aiLevel );
  if( !aiProfile )
  {
    aiProfile = NewObject< UAIProfile >( this, UAIProfile::StaticClass() );
    aiProfile->ai.aiLevel = AILevel::AINone; // Default player controlled
  }

  // Make sure that the gamemode is created.. if it isn't we can't set the color
  AWryvGameMode *gm = (AWryvGameMode*)GetWorld()->GetAuthGameMode();
  if( !gm )
  {
    error( FS( "GameMode not initialized, cannot find ref team colors" ) );
    return;
  }
  if( TeamNumber < gm->TeamColors.Num() )
  {
    FLinearColor color = gm->TeamColors[ TeamNumber ];
    SetMeshColor( Mesh, this, "TeamColor", color );
  }
}


