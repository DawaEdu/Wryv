#pragma once

#include "SolidWidget.h"

class StatusBar : public SolidWidget
{
public:
  TextWidget* Text;
  StatusBar( FLinearColor bkgColor ) : SolidWidget( "status bar", bkgColor )
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
