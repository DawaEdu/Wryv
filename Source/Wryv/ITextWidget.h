#pragma once

#include "ImageWidget.h"
#include "TextWidget.h"

class TextWidget;

class ITextWidget : public ImageWidget
{
  bool FixedSize; // the background graphic can have fixed size or
  // it can wrap/house the text in the textwidget inside
protected:
  TextWidget* Text;
public:
  ITextWidget( FString name, UTexture* pic, FString ftext, Alignment textAlignment, UFont* font=0, float scale=1.f );
  ITextWidget( FString name, UTexture* pic, FVector2D size, FString ftext, Alignment textAlignment, UFont* font=0, float scale=1.f );
  virtual void ClearExtras();
  void SetText( FString text, int alignment );
  void SetText( FString text );
  void SetText( int v );
  FString GetText(){ return Text->Text; }
  // on redraw, remeasure the bounds to wrap the contained text
  virtual void render( FVector2D offset ) override;
};

