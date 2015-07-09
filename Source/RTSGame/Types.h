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

  // Items.
  ITEMSPEEDUP       UMETA(DisplayName = "Item Speedup"),
  ITEMATTACKSTR     UMETA(DisplayName = "Item Attack Strength"),
  ITEMDEFENSEUP     UMETA(DisplayName = "Item Defense"),

  // Some kind of widget or selector
  DOODAD            UMETA(DisplayName = "Doodad"),
  UI_ICON           UMETA(DisplayName = "UI Icon"),

  MAX               UMETA(DisplayName = "Max"),
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

inline FString GetEnumName( Types enumValue )
{
  const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("Types"), true);
  return EnumPtr->GetDisplayNameText(enumValue).ToString();
}
