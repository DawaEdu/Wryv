#pragma once

#include "GameFramework/Pawn.h"
#include "Avatar.generated.h"

UCLASS()
class RTSGAME_API AAvatar : public APawn
{
  GENERATED_BODY()

public:
  // Sets default values for this character's properties
  AAvatar();

  // Called to bind functionality to input
  virtual void SetupPlayerInputComponent( UInputComponent* InputComponent ) override;
  void debug( int slot, FColor color, FString mess );
  void MouseClicked();
  void MouseRightClicked();
  void MoveForward( float amount );
  void MoveBack( float amount );
  void MoveRight( float amount );
  void MoveLeft( float amount );

  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  // Called every frame
  virtual void Tick( float DeltaSeconds ) override;

};
