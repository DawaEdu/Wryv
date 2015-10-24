#pragma once

#include "ImageWidget.h"
#include "Border.h"

class ImageBorder : public ImageWidget
{
  Border* border;
public:
  ImageBorder( FString name, UTexture* tex, float thickness, FLinearColor borderColor );
  virtual void SetTexture( UTexture* tex ) override;
};


