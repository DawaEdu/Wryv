#pragma once

#include "GlobalFunctions.h"
#include "Types.generated.h"

// Spawnable unit types
UENUM()
enum Types
{
  NOTHING = 0           UMETA(DisplayName = "NOTHING"),

  UNITSPHERE            UMETA(DisplayName = "Sphere"),
  UNITBOX               UMETA(DisplayName = "Box"),
  UNITEDGE              UMETA(DisplayName = "Edge"),

  // Units.
  UNITPEASANT           UMETA(DisplayName = "Peasant"),
  UNITFOOTMAN           UMETA(DisplayName = "Footman"),
  UNITARCHER            UMETA(DisplayName = "Archer"),
  UNITMAGE              UMETA(DisplayName = "Mage"),

  // Resources.
  RESTREE               UMETA(DisplayName = "Resource Tree"),
  RESSTONE              UMETA(DisplayName = "Resource Stone"),
  RESGOLDMINE           UMETA(DisplayName = "Resource Goldmine"),

  // Buildings.
  BLDGFARM              UMETA(DisplayName = "Building Farm"),
  BLDGBARRACKS          UMETA(DisplayName = "Building Barracks"),
  BLDGTOWNHALL          UMETA(DisplayName = "Building Townhall"),
  BLDGITEMSHOP          UMETA(DisplayName = "Building Itemshop"),

  // Spells.
  SPELLFIRE             UMETA(DisplayName = "Spell Fire"),
  SPELLICE              UMETA(DisplayName = "Spell Ice"),
  SPELLLIGHTNING        UMETA(DisplayName = "Spell Lightning"),

  SPELLFIRESPLASH       UMETA(DisplayName = "Spell Fire Splash"),
  SPELLICESPLASH        UMETA(DisplayName = "Spell Ice Splash"),
  SPELLLIGHTNINGSPLASH  UMETA(DisplayName = "Spell Lightning Splash"),

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
  UIATTACKSELECTOR      UMETA(DisplayName = "UI Attack Selector"),
  UIFOLLOWSELECTOR      UMETA(DisplayName = "UI Follow Selector"),
  UISHOPPATRONSELECTOR  UMETA(DisplayName = "UI Shop Patron Selector"),

  EXPLOSION1            UMETA(DisplayName = "Explosion 1"),
  EXPLOSION2            UMETA(DisplayName = "Explosion 2"),
  EXPLOSION3            UMETA(DisplayName = "Explosion 3"),

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
  return Between( b, RESTREE, RESGOLDMINE );
}
inline bool IsItem( Types b ) {
  return Between( b, ITEMSPEEDUP, ITEMDEFENSEUP );
}
inline bool IsSpell( Types b ) {
  return Between( b, SPELLFIRE, SPELLLIGHTNINGSPLASH );
}
inline bool IsAction( Types b ) {
  return Between( b, ACTIONMOVE, ACTIONDOUBLEDAMAGE );
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

