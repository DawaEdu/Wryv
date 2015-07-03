#include "RTSGame.h"
#include "Avatar.h"

// Sets default values
AAvatar::AAvatar()
{
  // Set this character to call Tick() every frame.
  // Turn this off to improve performance if you don't need it.
  PrimaryActorTick.bCanEverTick = true;
  UE_LOG( LogTemp, Log, TEXT("HELLO!!!!!!!!!!!!!!!!!!!!!!!!!!") );
  //GetCapsuleComponent()->SetCollisionEnabled( ECollisionEnabled::NoCollision );
  //GetGravityDirection
}

void AAvatar::SetupPlayerInputComponent( UInputComponent* InputComponent )
{
  check( InputComponent );
  Super::SetupPlayerInputComponent( InputComponent );
  
  InputComponent->BindAxis( "Forward", this, &AAvatar::MoveForward );
  InputComponent->BindAxis( "Back", this, &AAvatar::MoveBack );
  InputComponent->BindAxis( "Right", this, &AAvatar::MoveRight );
  InputComponent->BindAxis( "Left", this, &AAvatar::MoveLeft );
  
  InputComponent->BindAction( "MouseClickedLMB", IE_Pressed,
    this, &AAvatar::MouseClicked );
  InputComponent->BindAction( "MouseClickedRMB", IE_Pressed,
    this, &AAvatar::MouseRightClicked );
  
  UE_LOG( LogTemp, Log, TEXT("AAvatar::SetupPlayerInputComponent()") );
}

void AAvatar::debug( int slot, FColor color, FString mess )
{
	if( GEngine )
	{
		GEngine->AddOnScreenDebugMessage( slot, 5.f, color, mess );
	}
}

void AAvatar::MouseClicked()
{
  UE_LOG( LogTemp, Log, TEXT("AAvatar::MouseClicked()") );
  APlayerController* PController = GetWorld()->GetFirstPlayerController();
  AHUD* hud = Cast<AHUD>( PController->GetHUD() );
  
  FVector2D mouse;
  PController->GetMousePosition( mouse.X, mouse.Y );
  debug( 0, FColor::Yellow,
    FString::Printf( TEXT("mouse @ %f %f"), mouse.X, mouse.Y ) );
	
  // Perform trace to retrieve hit info
  FHitResult hit( ForceInit );
  if( PController->GetHitResultAtScreenPosition(
    mouse, ECollisionChannel::ECC_EngineTraceChannel1, true, hit ) )
  {
    AActor* actor = hit.GetActor();
    if( actor )
    {
      UE_LOG( LogTemp, Warning, TEXT("%s"), *actor->GetName() );
    }
    else
    {
      UE_LOG( LogTemp, Warning, TEXT("didn't hit") );
    }
  }
  
  //GetWorld()->LineTraceSingle( Hit, StartTrace, EndTrace,
  //  ECollisionChannel::ECC_EngineTraceChannel1, TraceParams ); // simple trace function
  //FString fs = FString::Printf( TEXT( "Clicked on %s" ), actor->GetName() );
  //UE_LOG( LogTemp, Warning, TEXT("%s %d"), fs, 5 );
  //FString fs = FString::Printf( TEXT( "%f" ), 47.f );
}

void AAvatar::MouseRightClicked()
{
  APlayerController* PController = GetWorld()->GetFirstPlayerController();
  AHUD* hud = Cast<AHUD>( PController->GetHUD() );
}

void AAvatar::MoveForward( float amount )
{
  //if( inventoryShowing ) return;
  // Don't enter the body of this function if Controller is
  // not set up yet, or if the amount to move is equal to 0 
  if( Controller && amount )
  {
    FVector fwd = GetActorForwardVector();
    // we call AddMovementInput to actually move the player
    // by `amount` in the `fwd` direction
    AddMovementInput( fwd, amount );
  }
}

void AAvatar::MoveBack( float amount )
{
  //if( inventoryShowing ) return;
  // Don't enter the body of this function if Controller is
  // not set up yet, or if the amount to move is equal to 0 
  if( Controller && amount )
  {
    FVector back = -GetActorForwardVector();
    // we call AddMovementInput to actually move the player
    // by `amount` in the `fwd` direction
    AddMovementInput( back, amount );
  }
}

void AAvatar::MoveRight( float amount )
{
  //if( inventoryShowing ) return;
  if( Controller && amount )
  {
    FVector right = GetActorRightVector();
    AddMovementInput( right, amount );
  }
}

void AAvatar::MoveLeft( float amount )
{
  if( Controller && amount )
  {
    FVector left = -GetActorRightVector();
    AddMovementInput( left, amount );
  }
}

// Called when the game starts or when spawned
void AAvatar::BeginPlay()
{
  //Super::BeginPlay();
}

// Called every frame
void AAvatar::Tick( float DeltaTime )
{
  //Super::Tick( DeltaTime );
}

