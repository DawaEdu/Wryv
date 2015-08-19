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
  ITextWidget( FString name, UTexture* pic, FString ftext, int textAlignment, UFont* font=0, float scale=1.f );
  ITextWidget( FString name, UTexture* pic, FVector2D size, FString ftext, int textAlignment, UFont* font=0, float scale=1.f );
  void Set( FString text );
  void Set( int v );
// on redraw, remeasure the bounds to wrap the contained text
  virtual void render( FVector2D offset ) override;
};

