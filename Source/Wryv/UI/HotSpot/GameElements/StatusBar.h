#pragma once

#include "UI/HotSpot/Elements/Solid.h"

class StatusBar : public Solid
{
public:
  TextWidget* Text;
  StatusBar( FLinearColor bkgColor ) : Solid( "status bar", bkgColor )
  {
    Align = Bottom | HFull;
    Text = new TextWidget( "status text" );
    Add( Text );
  }

  void Set( FString text )
  {
    Text->Set( text );
  }
};
