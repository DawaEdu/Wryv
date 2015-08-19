#pragma once

#include "SolidWidget.h"

class ProgressBar : public SolidWidget
{
public:
  SolidWidget* progress;
  ProgressBar( FString name, FLinearColor backgroundColor,
    FLinearColor foregroundColor, FVector2D size ) :
    SolidWidget( name, size, backgroundColor )
  {
    progress = new SolidWidget( "foreground progress", foregroundColor );
    Add( progress );
  }

  void Set( float percent )
  {
    progress->Size.X = percent * Size.X;
  }
};