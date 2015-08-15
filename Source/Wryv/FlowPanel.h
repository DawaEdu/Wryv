#pragma once

#include "ImageWidget.h"

class FlowPanel : public ImageWidget
{
public:
  int Rows, Cols;
  FlowPanel( FString name, UTexture* tex, int rows, int cols, FVector2D size ) :
    ImageWidget( name, tex, size ), Rows( rows ), Cols( cols )
  {
  }
  void reflow(){ reflow( Size ); }
  void reflow( FVector2D size )
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

  virtual void Move( float t )
  {
    reflow( Size );
  }

  void Set( set<AGameObject*> objects )
  {
    // Change rows/cols.
    Cols = Rows = ceilf( sqrtf( objects.size() ) ) ;
    Clear();
    if( !Cols ) return; //empty

    for( AGameObject* go : objects )
      Add( new ImageWidget( go->Stats.Name, go->Stats.Portrait ) ) ;
    if( Rows && Cols ) reflow();
  }
};


