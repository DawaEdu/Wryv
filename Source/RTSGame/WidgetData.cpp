#include "RTSGame.h"
#include "WidgetData.h"

FString FWidgetData::ToString()
{
  return FString::Printf( TEXT( "%s: %s (%f, %f) (%f, %f)" ),
    *GetEnumName( Type ), *Label,
    Pos.X, Pos.Y, Size.X, Size.Y );
}
