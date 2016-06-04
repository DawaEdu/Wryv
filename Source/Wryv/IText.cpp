#include "Wryv.h"

#include "IText.h"

IText::IText( FString name, UTexture* pic, FString ftext, Alignment textAlignment, 
  UFont* font, float scale ) : ImageHS( name, pic )
{
  if( pic ) Name += pic->GetName();
  SetName( Name );
  Text = new TextWidget( ftext, Alignment::TopLeft, font, scale );
  Text->Align = textAlignment;
  Add( Text );
  Pad = FVector2D( 8,8 );
  // assumes size of texture when not fixed size
  FixedSize = 0;// when size not supplied, assumed to wrap the textwidget inside
}

IText::IText( FString name, UTexture* pic, FVector2D size, FString ftext, Alignment textAlignment,
  UFont* font, float scale ) : ImageHS( pic, size )
{
  Name = name;
  if( pic ) Name += pic->GetName();
  SetName( Name );
  Text = new TextWidget( ftext, Alignment::TopLeft, font, scale );
  Text->Align = textAlignment;
  Add( Text );
  Pad = FVector2D( 8,8 );
  FixedSize = 1;  // size supplied in ctor maintained
}

void IText::SetText( FString text, int alignment )
{
  Text->Set( text );
  Text->Align = alignment;
  dirty = 1;
}

void IText::SetText( FString text )
{
  SetText( text, Text->Align );
}

void IText::SetText( int v )
{
  SetText( FS( "%d", v ) );
}

// on redraw, remeasure the bounds to wrap the contained text
void IText::render( FVector2D offset )
{
  if( hidden ) return;

  // Re-measure the text also
  if( dirty && !FixedSize )
  {
    Text->Measure();
    recomputeSizeToContainChildren();
    dirty = 0;
  }

  ImageHS::render( offset );
}
