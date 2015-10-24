#include "Wryv.h"

#include "FlyCam.h"
#include "Minimap.h"
#include "TheHUD.h"
#include "WryvGameInstance.h"

UTexture* Minimap::MinimapTexture = 0;

Minimap::Minimap( float borderThickness, FLinearColor borderColor ) : 
  ImageBorder( "minimap", MinimapTexture, borderThickness, borderColor )
{
  OnMouseDownLeft = [this](FVector2D mouse) -> EventCode {
    Game->flycam->SetCameraPosition( mouse / Size );
    return Consumed;
  };
  OnMouseDragLeft = [this](FVector2D mouse) -> EventCode {
    Game->flycam->SetCameraPosition( mouse / Size );
    return Consumed;
  };
}

void Minimap::render( FVector2D offset )
{
  ImageBorder::render( offset );
  // Convert groundIntnPts to pts on screen in screenspace of thing
  //for( int i = 0; i < pts.size()-1; i++ )
  //  DrawDebugLine( UGameplayStatics::GetWorld(), pts[i], pts[i+1], FColor( 100, 100, 100, 100 ) );
  //if( pts.size() > 1 )
  //  DrawDebugLine( UGameplayStatics::GetWorld(), pts[pts.size()-2], pts[pts.size()-1], FColor( 100, 100, 100, 100 ) );

}



