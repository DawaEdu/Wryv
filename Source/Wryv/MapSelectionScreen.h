#pragma once

#include "Screen.h"

class MapSelectionScreen : public Screen
{
public:
  ImageWidget *Logo;
  StackPanel *MapFiles;
  FVector2D MapFileEntrySize;
  ImageWidget *Thumbnail;
  ITextWidget *OKButton;
  
  UTexture* MapSlotEntryBkg;
  ITextWidget* Selected;
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
    Logo = new ImageWidget( "Logo", logoTex );
    Logo->Align = TopLeft;
    Logo->Margin = FVector2D( 75, 50 );
    Add( Logo );
    
    // The stack of menu items.
    MapFiles = new StackPanel( "Map files", mapFilesBkg );
    MapFiles->Align = TopLeft;
    MapFiles->Margin = FVector2D( 12, 12 );
    MapFiles->Pad = FVector2D( 8,8 );
    Add( MapFiles );

    Thumbnail = new ImageWidget( "Thumbnail", thumbnailTex );
    Thumbnail->Align = CenterRight;
    Thumbnail->Margin = FVector2D( 80, 0 );
    Add( Thumbnail );

    OKButton = new ITextWidget( "Okbutton", mapSlotEntryBkg, entrySize, "OK", CenterCenter );
    OKButton->Align = BottomRight;
    OKButton->Margin = FVector2D( 20, 10 );
    
    Add( OKButton );
  }

  // Adds a slot
  ITextWidget* AddText( FString ftext, int textAlignment )
  {
    ITextWidget* text = new ITextWidget( FS("mss text %s",*ftext),
      MapSlotEntryBkg, MapFileEntrySize, ftext, textAlignment );
    text->OnMouseDownLeft = [this,text](FVector2D mouse){
      Select( text );
      return Consumed;
    };
    MapFiles->StackBottom( text );
    return text;
  }

  // Make widget selected
  void Select( ITextWidget *widget )
  {
    LOG( "Selected %s", *widget->GetText() ) ;

    // turn prev selected back to white
    if( Selected )  Selected->Color = FColor::White;
    Selected = widget;
    Selected->Color = FColor::Yellow;
  }
};

