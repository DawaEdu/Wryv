#pragma once

#include "Enums.generated.h"

UENUM()
enum Abilities
{
  NotSet      UMETA(DisplayName = "NotSet"),
  Movement    UMETA(DisplayName = "Move"),
  Stop        UMETA(DisplayName = "Stop"),
  Attack      UMETA(DisplayName = "Attack"),
  HoldGround  UMETA(DisplayName = "Hold Ground"),
};

UENUM()
enum AILevel
{
  AINone      UMETA(DisplayName = "AINone"),
  AIEasy      UMETA(DisplayName = "AIEasy"),
  AIMedium    UMETA(DisplayName = "AIMedium"),
  AIHard      UMETA(DisplayName = "AIHard")
};

UENUM()
enum GameStates
{
  Title = 0           UMETA(DisplayName = "Title"),
  MapSelect           UMETA(DisplayName = "MapSelect"),
  Running             UMETA(DisplayName = "Running"),
};

UENUM()
enum UISounds
{
  BuildingPlaced = 0  UMETA(DisplayName = "BuildingPlaced"),
  Error               UMETA(DisplayName = "Error"),
  Click               UMETA(DisplayName = "Click"),
  Music               UMETA(DisplayName = "Music"),
};





