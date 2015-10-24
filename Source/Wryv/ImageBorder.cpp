#include "Wryv.h"

#include "ImageBorder.h"

ImageBorder::ImageBorder( FString name, UTexture* tex, float thickness, FLinearColor borderColor ) :
  ImageWidget( name, tex )
{
  // Size determined by ImageWidget. Border will cover padding.
  FBox2DU box( FVector2D(0.f,0.f), Size );
  border = new Border( FS( "Border for %s", *name ), box, thickness, borderColor );
  Add( border );
}

void ImageBorder::SetTexture( UTexture* tex )
{
  ImageWidget::SetTexture( tex );
  FBox2DU box( FVector2D(0.f,0.f), Size );
  border->Set( box );
}


