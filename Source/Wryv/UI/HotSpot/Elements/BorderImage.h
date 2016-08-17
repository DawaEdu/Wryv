#pragma once

#include "UI/HotSpot/Elements/Image.h"
#include "UI/HotSpot/Elements/Border.h"

class BorderImage : public Image
{
  Border* border;
public:
  BorderImage( FString name, UTexture* tex, float thickness, FLinearColor borderColor );
  virtual void SetTexture( UTexture* tex ) override;
};


