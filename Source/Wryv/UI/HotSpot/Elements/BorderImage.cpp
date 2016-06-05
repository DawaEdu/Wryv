#include "Wryv.h"

#include "UI/HotSpot/Elements/BorderImage.h"

BorderImage::BorderImage( FString name, UTexture* tex, float thickness, FLinearColor borderColor ) :
  ImageHS( name, tex )
{
  // Size determined by ImageHS. Border will cover padding.
  FBox2DU box( FVector2D(0.f,0.f), Size );
  border = new Border( FS( "Border for %s", *name ), box, thickness, borderColor );
  Add( border );
}

void BorderImage::SetTexture( UTexture* tex )
{
  ImageHS::SetTexture( tex );
  FBox2DU box( FVector2D(0.f,0.f), Size );
  border->Set( box );
}


