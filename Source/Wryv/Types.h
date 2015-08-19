#pragma once

#include "Types.generated.h"

// Spawnable unit types
UENUM()
enum Types
{
  NOTHING = 0       UMETA(DisplayName = "NOTHING"),

  UNITSPHERE        UMETA(DisplayName = "Sphere"),
  UNITBOX           UMETA(DisplayName = "Box"),
  UNITEDGE          UMETA(DisplayName = "Edge"),

  // Units.
  UNITPEASANT       UMETA(DisplayName = "Peasant"),
  UNITFOOTMAN       UMETA(DisplayName = "Footman"),
  UNITARCHER        UMETA(DisplayName = "Archer"),
  UNITMAGE          UMETA(DisplayName = "Mage"),

  // Resources.
  RESTREE           UMETA(DisplayName = "Resource Tree"),
  RESSTONE          UMETA(DisplayName = "Resource Stone"),
  RESGOLDMINE       UMETA(DisplayName = "Resource Goldmine"),

  // Buildings.
  BLDGFARM          UMETA(DisplayName = "Building Farm"),
  BLDGBARRACKS      UMETA(DisplayName = "Building Barracks"),
  BLDGTOWNHALL      UMETA(DisplayName = "Building Townhall"),
  BLDGITEMSHOP      UMETA(DisplayName = "Building Itemshop"),

  // Spells.
  SPELLFIRE         UMETA(DisplayName = "Spell Fire"),
  SPELLICE          UMETA(DisplayName = "Spell Ice"),
  SPELLLIGHTNING    UMETA(DisplayName = "Spell Lightning"),

  SPELLFIRESPLASH       UMETA(DisplayName = "Spell Fire Splash"),
  SPELLICESPLASH        UMETA(DisplayName = "Spell Ice Splash"),
  SPELLLIGHTNINGSPLASH  UMETA(DisplayName = "Spell Lightning Splash"),

  // Ability to boost own speed
  ABILITYMOVE          UMETA(DisplayName = "Ability Move"),
  ABILITYATTACK        UMETA(DisplayName = "Ability Attack"),
  ABILITYHOLDGROUND    UMETA(DisplayName = "Ability Hold Ground"),
  ABILITYHASTE         UMETA(DisplayName = "Ability Haste"),
  ABILITYDOUBLEDAMAGE  UMETA(DisplayName = "Ability 2x damage" ),

  // Items.
  ITEMSPEEDUP          UMETA(DisplayName = "Item Speedup"),
  ITEMATTACKSTR        UMETA(DisplayName = "Item Attack Strength"),
  ITEMDEFENSEUP        UMETA(DisplayName = "Item Defense"),

  // Some kind of widget or selector
  UISELECTOR            UMETA(DisplayName = "UI Selector"),
  UIATTACKSELECTOR      UMETA(DisplayName = "UI Attack Selector"),
  UIFOLLOWSELECTOR      UMETA(DisplayName = "UI Follow Selector"),
  UISHOPPATRONSELECTOR  UMETA(DisplayName = "UI Shop Patron Selector"),

  MAX                   UMETA(DisplayName = "Max"),
};

UENUM()
enum GameStates
{
  Title = 0         UMETA(DisplayName = "Title"),
  MapSelect         UMETA(DisplayName = "MapSelect"),
  Running           UMETA(DisplayName = "Running"),
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

inline FString GetEnumName( TCHAR* enumType, int enumValue )
{
  TCHAR* t = L"Types";
  const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, enumType, true);
  return EnumPtr->GetDisplayNameText(enumValue).ToString();
}

inline FString GetTypesName( Types enumValue )
{
  const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT( "Types" ), true);
  return EnumPtr->GetDisplayNameText(enumValue).ToString();
}

