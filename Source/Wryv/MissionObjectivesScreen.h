#pragma once

#include "Screen.h"

// Displays mission objectives on screen
class MissionObjectivesScreen : public Screen
{
  UTexture *SlotBkg;
  StackPanel *Objectives;
public:
  MissionObjectivesScreen( UTexture *bkg, UTexture *slotBkg,
    FVector2D slotSize, FVector2D pad ) :
    Screen( "MissionObjectives" )
  {
    Objectives = new StackPanel( "Objectives stackpanel", bkg );
  }

  // Add a series of mission objectives here
  ITextWidget* AddText( FString text, int align )
  {
    ITextWidget *tw = new ITextWidget( FS("MBText `%s`",*text), SlotBkg, FVector2D( 100, 40 ), text, CenterCenter );
    tw->Margin = FVector2D( 10, 0 );
    tw->Align = CenterLeft;
    return Add( tw );
  }
};


