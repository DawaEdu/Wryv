#pragma once

#include "GlobalFunctions.h"
#include "Types.generated.h"

// Spawnable unit types
UENUM()
enum Types
{
  NOTHING = 0           UMETA(DisplayName = "NOTHING"),

  SHAPESPHERE           UMETA(DisplayName = "Sphere"),
  SHAPECUBE             UMETA(DisplayName = "Cube"),
  SHAPEEDGE             UMETA(DisplayName = "Edge"),

  // Units.
  UNITPEASANT           UMETA(DisplayName = "Unit Peasant"),
  UNITFOOTMAN           UMETA(DisplayName = "Unit Footman"),
  UNITARCHER            UMETA(DisplayName = "Unit Archer"),
  UNITMAGE              UMETA(DisplayName = "Unit Mage"),

  // Resources.
  RESLUMBER             UMETA(DisplayName = "Resource Lumber"),
  RESSTONE              UMETA(DisplayName = "Resource Stone"),
  RESGOLD               UMETA(DisplayName = "Resource Goldmine"),

  // Buildings.
  BLDGFARM              UMETA(DisplayName = "Building Farm"),
  BLDGBARRACKS          UMETA(DisplayName = "Building Barracks"),
  BLDGTOWNHALL          UMETA(DisplayName = "Building Townhall"),
  BLDGITEMSHOP          UMETA(DisplayName = "Building Itemshop"),

  TOMBSTONE             UMETA(DisplayName = "Tombstone"),
  GROUNDPLANE           UMETA(DisplayName = "Ground Plane"),

  // ACTION to boost own speed
  ACTIONMOVE            UMETA(DisplayName = "Action Move"),
  ACTIONATTACK          UMETA(DisplayName = "Action Attack"),
  ACTIONHOLDGROUND      UMETA(DisplayName = "Action Hold Ground"),
  ACTIONHASTE           UMETA(DisplayName = "Action Haste"),
  ACTIONDOUBLEDAMAGE    UMETA(DisplayName = "Action 2x damage" ),

  // Items.
  ITEMSPEEDUP           UMETA(DisplayName = "Item Speedup"),
  ITEMATTACKSTR         UMETA(DisplayName = "Item Attack Strength"),
  ITEMDEFENSEUP         UMETA(DisplayName = "Item Defense"),

  // Some kind of widget or selector
  UISELECTOR            UMETA(DisplayName = "UI Selector"),
  UIFLAGWAYPOINT        UMETA(DisplayName = "UI Flag Waypoint"),

  // Weapons.
  PROJWEAPONARROW       UMETA(DisplayName = "Arrow Projectile"),
  PROJFIRE              UMETA(DisplayName = "Fire Projectile"),
  PROJICE               UMETA(DisplayName = "Ice Projectile"),
  PROJLIGHTNING         UMETA(DisplayName = "Lightning Projectile"),

  EXPLWHITE             UMETA(DisplayName = "Explosion White"),
  EXPLYELLOW            UMETA(DisplayName = "Explosion Yellow"),
  EXPLARROWDUST         UMETA(DisplayName = "Explosion Arrow Dust"),
  EXPLFIRESPLASH        UMETA(DisplayName = "Explosion Fire Splash"),
  EXPLICESPLASH         UMETA(DisplayName = "Explosion Ice Splash"),
  EXPLLIGHTNINGSPLASH   UMETA(DisplayName = "Explosion Lightning Splash"),

  MAX                   UMETA(DisplayName = "Max"),
};

UENUM()
enum GameStates
{
  Title = 0             UMETA(DisplayName = "Title"),
  MapSelect             UMETA(DisplayName = "MapSelect"),
  Running               UMETA(DisplayName = "Running"),
};

inline bool Between( int v, int lo, int hi ) {
  return lo <= v && v <= hi ;
}
inline bool IsUnit( Types b ) {
  return Between( b, UNITPEASANT, UNITARCHER );
}
inline bool IsBuilding( Types b ) {
  return Between( b, BLDGFARM, BLDGTOWNHALL );
}
inline bool IsResource( Types b ) {
  return Between( b, RESLUMBER, RESGOLD );
}
inline bool IsItem( Types b ) {
  return Between( b, ITEMSPEEDUP, ITEMDEFENSEUP );
}
inline bool IsProjectile( Types b ) {
  return Between( b, PROJWEAPONARROW, PROJLIGHTNING );
}
inline bool IsExplosion( Types b ) {
  return Between( b, EXPLWHITE, EXPLLIGHTNINGSPLASH );
}
inline bool IsAction( Types b ) {
  return Between( b, ACTIONMOVE, ACTIONDOUBLEDAMAGE );
}
inline bool IsShape( Types b ) {
  return Between( b, SHAPESPHERE, SHAPEEDGE );
}
inline bool IsValidType( Types b ){
  return Between( b, 0, Types::MAX-1 );
}

inline FString GetEnumName( TCHAR* enumType, int enumValue )
{
  const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, enumType, true);
  return EnumPtr->GetDisplayNameText(enumValue).ToString();
}

inline FString GetTypesName( Types enumValue )
{
  if( enumValue < 0 || enumValue >= Types::MAX )
  {
    error( FS( "GetTypesName: enumValue %d OOB", (int)enumValue ) );
    return FS( "Out of bounds type %d", (int)enumValue );
  }
  const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT( "Types" ), true);
  return EnumPtr->GetDisplayNameText(enumValue).ToString();
}

