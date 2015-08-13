#pragma once

#include "ImageWidget.h"
#include "TextWidget.h"

class TextWidget;

class ITextWidget : public ImageWidget
{
  TextWidget* Text;
  bool FixedSize; // the background graphic can have fixed size or
  // it can wrap/house the text in the textwidget inside
public:
  FString GetText(){ return Text->Text; }
  ITextWidget( FString name, UTexture* pic, FString ftext, int textAlignment, 
    UFont* font=0, float scale=1.f ) :
    ImageWidget( name, pic )
  {
    if( pic ) Name += pic->GetName();
    SetName( Name );
    Text = new TextWidget( ftext, font, scale );
    Text->Align = textAlignment;
    Add( Text );
    Pad = FVector2D( 8,8 );
    // assumes size of texture when not fixed size
    FixedSize = 0;// when size not supplied, assumed to wrap the textwidget inside
  }

  ITextWidget( FString name, UTexture* pic, FVector2D size, FString ftext, int textAlignment, 
    UFont* font=0, float scale=1.f ) :
    ImageWidget( pic, size )
  {
    Name = name;
    if( pic ) Name += pic->GetName();
    SetName( Name );
    Text = new TextWidget( ftext, font, scale );
    Text->Align = textAlignment;
    Add( Text );
    Pad = FVector2D( 8,8 );
    FixedSize = 1;// size supplied in ctor maintained
  }

  void Set( FString text )
  {
    Text->Set( text );
    dirty = 1;
  }
  void Set( int v )
  {
    Set( FS( "%d", v ) );
  }

  // on redraw, remeasure the bounds to wrap the contained text
  virtual void render( FVector2D offset ) override
  {
    // Re-measure the text also
    if( dirty && !FixedSize )
    {
      Text->Measure();
      recomputeSizeToContainChildren();
      dirty = 0;
    }

    ImageWidget::render( offset );
  }
};

