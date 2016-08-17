#include "Wryv.h"

#include "UI/HotSpot/Elements/BorderImage.h"

BorderImage::BorderImage( FString name, UTexture* tex, float thickness, FLinearColor borderColor ) :
  Image( name, tex )
{
  // Size determined by Image. Border will cover padding.
  FBox2DU box( FVector2D(0.f,0.f), Size );
  border = new Border( FS( "Border for %s", *name ), box, thickness, borderColor );
  Add( border );
}

void BorderImage::SetTexture( UTexture* tex )
{
  Image::SetTexture( tex );
  FBox2DU box( FVector2D(0.f,0.f), Size );
  border->Set( box );
}


