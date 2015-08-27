#include "Wryv.h"
#include "FlowPanel.h"

#include "ImageWidget.h"

FlowPanel::FlowPanel( FString name, UTexture* bgTex, int rows, int cols, FVector2D size ) :
  ImageWidget( name, bgTex, size ), Rows( rows ), Cols( cols )
{
}

void FlowPanel::reflow()
{
  reflow( Size );
}

void FlowPanel::reflow( FVector2D size )
{
  Size = size;
  FVector2D ColsRows( Cols, Rows );
  FVector2D tileDims = ( size - (Pad*ColsRows) ) / ColsRows;
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
    FVector2D pos = Pad/2 + ( Pad + tileDims ) * FVector2D( col, row );
    children[i]->Margin = pos;
    children[i]->Size = tileDims;
  }
}

// re-flow after each add
void FlowPanel::PostAdd()
{
  HotSpot::PostAdd();
  reflow();
}
  
void FlowPanel::Set( set<AGameObject*> objects )
{
  // Change rows/cols.
  Cols = Rows = ceilf( sqrtf( objects.size() ) ) ;
  Clear();
  if( !Cols ) return; //empty

  for( AGameObject* go : objects )
    Add( new ImageWidget( go->Stats.Name, go->Stats.Portrait ) ) ;
  if( Rows && Cols ) reflow();
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


