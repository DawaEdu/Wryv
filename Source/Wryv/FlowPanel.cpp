#include "Wryv.h"

#include "FlowPanel.h"
#include "ImageWidget.h"

FlowPanel::FlowPanel( FString name, UTexture* bgTex, int rows, int cols, FVector2D size ) :
  ImageWidget( name, bgTex, size ), Rows( rows ), Cols( cols )
{
}

void FlowPanel::Reflow()
{
  if( !Rows || !Cols )
  {
    return;
  }

  FVector2D ColsRows( Cols, Rows );
  FVector2D tileDims = ( Size - (Pad*ColsRows) ) / ColsRows;
  //info( FS( "%s tileDims = %f %f", *Name, tileDims.X, tileDims.Y ) );
  
  // +---+---+---+
  // |   |   |   |
  // +---+---+---+
  // |   |   |   |
  // +---+---+---+
  //   0   1   2
  // Set new bounds for container
  for( int i = 0; i < children.size(); i++ )
  {
    int row = i / Cols;
    int col = i % Cols;
    FVector2D pos = Pad/2.f + ( Pad + tileDims ) * FVector2D( col, row );
    children[i]->Margin = pos;
    children[i]->Size = tileDims;
  }

  ImageWidget::Reflow();
}

// re-flow after each add
void FlowPanel::PostAdd()
{
  HotSpot::PostAdd();
  Reflow();
}
  
void FlowPanel::render( FVector2D offset )
{
  if( hidden ) return;

  if( Tex )
  {
    ImageWidget::render( offset );
  }
  else
  {
    // Call base render immediately without trying to render tex
    HotSpot::render( offset );
  }
}


