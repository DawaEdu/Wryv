#include "Wryv.h"

#include "Util/GlobalFunctions.h"
#include "UE4/PlayerStartPosition.h"
#include "UE4/WryvGameMode.h"

APlayerStartPosition::APlayerStartPosition( const FObjectInitializer& PCIP ) : Super( PCIP )
{
  Mesh = PCIP.CreateDefaultSubobject<USkeletalMeshComponent>( this, TEXT( "flag1" ) );
  Mesh->AttachTo( GetRootComponent() );
  TeamNumber = 0;
}

void APlayerStartPosition::PostInitializeComponents()
{
  Super::PostInitializeComponents();
  aiProfile = Construct<UAIProfile>( aiLevel );
  if( !aiProfile )
  {
    aiProfile = Construct< UAIProfile >( UAIProfile::StaticClass() );
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


