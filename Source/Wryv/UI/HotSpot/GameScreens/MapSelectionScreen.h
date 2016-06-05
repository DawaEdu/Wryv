#pragma once

#include "UI/HotSpot/Elements/Screen.h"

class MapSelectionScreen : public Screen
{
public:
  ImageHS *Logo;
  StackPanel *MapFiles;
  FVector2D MapFileEntrySize;
  ImageHS *Thumbnail;
  IText *OKButton;
  
  UTexture* MapSlotEntryBkg;
  IText* Selected;
  UFont* Font;

  // Constructs the MapSelection Screen.
  MapSelectionScreen( UTexture* logoTex,
    UTexture* mapFilesBkg, UTexture* mapSlotEntryBkg,
    UTexture* thumbnailTex, FVector2D size, FVector2D entrySize, UFont* font ) :
    Screen( "MapSelectScreen", size ),
    MapFileEntrySize( entrySize ),
    MapSlotEntryBkg( mapSlotEntryBkg ),
    Selected( 0 ), Font( font )
  {
    // Throw in the title
    Logo = new ImageHS( "Logo", logoTex );
    Logo->Align = TopLeft;
    Logo->Margin = FVector2D( 75, 50 );
    Add( Logo );
    
    // The stack of menu items.
    MapFiles = new StackPanel( "Map files", mapFilesBkg );
    MapFiles->Align = TopLeft;
    MapFiles->Margin = FVector2D( 12, 12 );
    MapFiles->Pad = FVector2D( 8,8 );
    Add( MapFiles );

    Thumbnail = new ImageHS( "Thumbnail", thumbnailTex );
    Thumbnail->Align = CenterRight;
    Thumbnail->Margin = FVector2D( 80, 0 );
    Add( Thumbnail );

    OKButton = new IText( "Okbutton", mapSlotEntryBkg, entrySize, "OK", CenterCenter );
    OKButton->Align = BottomRight;
    OKButton->Margin = FVector2D( 20, 10 );
    
    Add( OKButton );
  }

  // Adds a slot
  IText* AddText( FString ftext, Alignment textAlignment )
  {
    IText* text = new IText( FS("mss text %s",*ftext),
      MapSlotEntryBkg, MapFileEntrySize, ftext, textAlignment );
    text->OnMouseDownLeft = [this,text](FVector2D mouse) -> EventCode {
      Select( text );
      return Consumed;
    };
    MapFiles->StackBottom( text, HCenter );
    return text;
  }

  // Make widget selected
  void Select( IText *widget )
  {
    LOG( "Selected %s", *widget->GetText() ) ;

    // turn prev selected back to white
    if( Selected )  Selected->Color = FColor::White;
    Selected = widget;
    Selected->Color = FColor::Yellow;
  }
};

