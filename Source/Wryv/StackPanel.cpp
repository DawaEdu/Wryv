#include "Wryv.h"
#include "StackPanel.h"

UTexture* StackPanel::StackPanelTexture = 0;

StackPanel::StackPanel( FString name ) : ImageWidget( name )
{
}
StackPanel::StackPanel( FString name, UTexture* bkg ) : ImageWidget( name, bkg )
{
}
StackPanel::StackPanel( FString name, UTexture* bkg, FLinearColor color ) : ImageWidget( name, bkg, color )
{
}
  
// Override base class function, to prevent warning when
// texture not provided
void StackPanel::render( FVector2D offset )
{
  if( hidden ) return;

  // Doesn't render the texture if it isn't set,
  // so that clear background is allowed
  if( Tex )  ImageWidget::render( offset );
  // render children
  HotSpot::render( offset );
}
