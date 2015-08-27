#include "Wryv.h"
#include "Minimap.h"
#include "WryvGameInstance.h"
#include "FlyCam.h"

UTexture* Minimap::MinimapTexture = 0;

Minimap::Minimap( float borderSize, FLinearColor borderColor ) : 
  ImageWidget( "minimap", MinimapTexture )
{
  FBox2DU box( FVector2D(0,0), Size );
  borders = new Border( "Minimap border", box, borderSize, borderColor );
  Add( borders );
  OnMouseDownLeft = [this](FVector2D mouse) -> EventCode {
    Game->flycam->SetCameraPosition( mouse / Size );
    return Consumed;
  };
  OnMouseDragLeft = [this](FVector2D mouse) -> EventCode {
    Game->flycam->SetCameraPosition( mouse / Size );
    return Consumed;
  };
}


