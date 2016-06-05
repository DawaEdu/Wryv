#pragma once

#include "UI/HotSpot/Elements/ImageHS.h"
#include "UI/HotSpot/Elements/Border.h"

class BorderImage : public ImageHS
{
  Border* border;
public:
  BorderImage( FString name, UTexture* tex, float thickness, FLinearColor borderColor );
  virtual void SetTexture( UTexture* tex ) override;
};


